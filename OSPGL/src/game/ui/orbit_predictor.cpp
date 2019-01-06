#include "orbit_predictor.h"

// Thread functions

static OrbitSnapshot simulate(OrbitSnapshot t0, double dt, SpaceSystem* sys)
{
	OrbitSnapshot out;

	NewtonState st;
	st.pos = t0.pos; st.delta = t0.delta;
	NewtonState t1 = sys->simulate_static(dt, t0.t, st);

	out.pos = t1.pos;
	out.delta = t1.delta;
	out.t = t0.t + dt;

	return out;
}

static void def_predictor(OrbitPredictor* owner)
{
	while (owner->predicting == true)
	{
		if (owner->def_future.size() != 0 && owner->def_future_clear == false)
		{
			// Predict one more point
			OrbitSnapshot prev = owner->def_future[owner->def_future.size() - 1];
			if (prev.t <= owner->system->time + owner->def_predictor_sets.future_points_time)
			{
				OrbitSnapshot next = simulate(prev, owner->def_predictor_sets.predictor_dt, owner->system);
				owner->mtx.lock();
				owner->def_future.push_back(next);
				owner->mtx.unlock();
			}



			// Clear old points
			if (owner->def_future.size() > 2)
			{
				size_t i;
				for (i = 0; i < owner->def_future.size(); i++)
				{
					if (owner->def_future[i].t >= owner->system->time)
					{
						break;
					}
				}

				owner->mtx.lock();
				// pop elements
				for (size_t j = 0; j < i; j++)
				{
					owner->def_future.pop_front();
				}

				owner->mtx.unlock();
			}

		}

	}

	return;
}




void OrbitPredictor::update_mesh()
{
	if (future_prev_points != def_future.size())
	{
		future_mesh.mh.destroy();
	}

	future_mesh.mh.vertices.clear();
	Vertex prev;
	prev.pos = glm::vec3(0, 0, 0);
	mtx.lock();
	for (size_t i = 0; i < def_future.size(); i+= def_predictor_sets.future_precision)
	{
		if (i > def_future.size() - 1)
		{
			i = def_future.size() - 1;
		}
		float perc = 1.0f - ((float)i / (float)def_future.size());

		glm::dvec3 p = def_frame.transform(def_future[i].pos, def_future[i].t);

		Vertex vert;
		vert.pos = p * 10e-9;
		vert.col = glm::vec3(perc * 0.6f + 0.4f, perc * 0.1f + 0.1f, perc * 0.1f + 0.1f);
		if (prev.pos == glm::vec3(0, 0, 0))
		{
			prev.pos = vert.pos;
			future_mesh.mh.vertices.push_back(vert);
		}
		else
		{
			future_mesh.mh.vertices.push_back(prev);
		}
		future_mesh.mh.vertices.push_back(vert);
		prev = vert;
	}
	mtx.unlock();

	future_mesh.draw_type = GL_LINES;
	future_mesh.mh.build_array();
	future_mesh.mh.upload(true);


	if (past_prev_points != def_past.size())
	{
		past_mesh.mh.destroy();
	}

	past_mesh.mh.vertices.clear();
	prev.pos = glm::vec3(0, 0, 0);
	for (size_t i = 0; i < def_past.size(); i++)
	{
		float perc = (float)i / (float)def_past.size();
		glm::dvec3 p = def_frame.transform(def_past[i].pos, def_past[i].t);
		Vertex vert;
		vert.pos = p * 10e-9;
		vert.col = glm::vec3(perc * 0.5f + 0.5f, perc * 0.5f + 0.5f, perc * 0.5f + 0.5f);
		if (prev.pos == glm::vec3(0, 0, 0))
		{
			prev.pos = vert.pos;
			past_mesh.mh.vertices.push_back(vert);
		}
		else
		{
			past_mesh.mh.vertices.push_back(prev);
		}
		past_mesh.mh.vertices.push_back(vert);
		prev = vert;
	}

	past_mesh.draw_type = GL_LINES;
	past_mesh.mh.build_array();
	past_mesh.mh.upload(true);
}

void OrbitPredictor::draw(glm::mat4 view, glm::mat4 proj)
{
	past_mesh.tform.pos = def_frame.center->last_state.pos / ORBIT_VIEW_SCALE;
	future_mesh.tform.pos = def_frame.center->last_state.pos / ORBIT_VIEW_SCALE;
	past_mesh.draw(view, proj);
	future_mesh.draw(view, proj);

	if (show_ui)
	{
		if (ui_manager.orbit_plotter)
		{
			ImGui::Begin("Orbit Predictor Settings", &ui_manager.orbit_plotter);

			ImGui::SameLine();
			if (ImGui::Button("Clear Past"))
			{
				def_past.clear();
			}
			ImGui::SameLine();
			if (ImGui::Button("Clear Future"))
			{
				mtx.lock();
				def_future.clear();
				def_future_clear = true;
				mtx.unlock();
			}

			float past_time = def_predictor_sets.past_points_time / 86400;
			float future_time = def_predictor_sets.future_points_time / 86400;
			//float precision = def_predictor_sets.predictor_dt;

			ImGui::DragFloat("Past Time", &past_time, 0.2f, 0.0f, 0.0f, "%.2f days");
			ImGui::DragFloat("Future Time", &future_time, 0.2f, 0.0f, 0.0f, "%.2f days");
			//ImGui::DragFloat("Precision", &precision, 0.1f);

			if (def_future.size() == 0)
			{
				ImGui::Text("No prediction");
			}
			else
			{
				mtx.lock();
				ImGui::Text("Predicted: %f days", (def_future[def_future.size() - 1].t - system->time) / 86400);
				mtx.unlock();
			}

			if (past_time < 0.2f)
			{
				past_time = 0.2f;
			}

			if (future_time < 0.2f)
			{
				future_time = 0.2f;
			}

			/*if (precision < 0.1f)
			{
				precision = 0.1f;
			}

			if (precision > 100.0f)
			{
				precision = 100.0f;
			}*/

			def_predictor_sets.past_points_time = past_time * 86400;
			def_predictor_sets.future_points_time = future_time * 86400;
			//def_predictor_sets.predictor_dt = precision;

			ImGui::Separator();
			ImGui::Text("Frame of Reference:");

			ImGui::BeginColumns("TEST", 2);
			ImGui::Text("Center:");
			for (size_t i = 0; i < system->bodies.size(); i++)
			{
				bool n = false;
				if (def_frame.center == system->bodies[i])
				{
					n = true;
				}
				ImGui::Checkbox(system->bodies[i]->id.c_str(), &n);
				if (n == true)
				{
					def_frame.center = system->bodies[i];
				}
			}

			ImGui::NextColumn();
			ImGui::Text("Settings:");
			ImGui::EndColumns();

			ImGui::End();
		}
	}
}

OrbitPredictor::OrbitPredictor(const OrbitPredictor &b)
{
	show_ui = b.show_ui;
	past_mesh = b.past_mesh;
	future_mesh = b.future_mesh;
	past_prev_points = b.past_prev_points;
	future_prev_points = b.future_prev_points;
	
	def_predictor_sets = b.def_predictor_sets;
	system = b.system;
	def_frame = b.def_frame;
	uid = b.uid;

	def_past = b.def_past;
	def_future = b.def_future;

	// TODO: Reinitialize from segments
	segments = b.segments;

	// Reinitialize threads
	open_threads();


}

OrbitPredictor::OrbitPredictor(SpaceSystem* system)
{
	this->system = system;
	// Open threads
	open_threads();
}

OrbitPredictor::~OrbitPredictor()
{
	close_threads();
}


void OrbitPredictor::close_threads()
{
	this->predicting = false;
	this->def_thread->join();
	delete this->def_thread;
}

void OrbitPredictor::open_threads()
{
	this->predicting = true;
	this->def_thread = new std::thread(def_predictor, this);
}

void OrbitPredictor::update(double dt, double t, NewtonState state)
{
	int count = def_predictor_sets.past_points_time / dt;
	this->def_past.push_back(OrbitSnapshot(state, t));

	// Remove a point to keep number of points constant
	if (this->def_past.size() >= count)
	{
		this->def_past.pop_front();
	}

	if (def_future_clear)
	{
		mtx.lock();
		this->def_future.clear();
		mtx.unlock();
	}

	if (this->def_future.size() == 0)
	{
		this->def_future.push_back(OrbitSnapshot(state, t));
		def_future_clear = false;
	}

	spdlog::get("OSP")->info("Point count: {}", this->def_future.size());

	update_mesh();
}


glm::dvec3 ReferenceFrame::transform(glm::dvec3 inertial, double t)
{
	double true_at = center->mean_to_true(center->time_to_mean(t));
	glm::dvec3 pos = center->to_state_at(true_at, t).pos;
	// Centered means that said body is at the origin
	glm::dvec3 diff = inertial - pos;

	// Apply rotational frame

	// (Diff must later be centered on the center body by the renderer
	// using a shader transform)
	return diff;
}


