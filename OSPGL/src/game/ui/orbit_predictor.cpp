#include "orbit_predictor.h"



void OrbitPredictor::update_mesh()
{
	if (future_prev_points != def_future.size())
	{
		future_mesh.mh.destroy();
	}


	future_mesh.mh.build_array();
	future_mesh.mh.upload(true);


	if (past_prev_points != def_past.size())
	{
		past_mesh.mh.destroy();
	}

	past_mesh.mh.vertices.clear();
	Vertex prev;
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
	past_mesh.draw(view, proj);
	future_mesh.draw(view, proj);
}

OrbitPredictor::OrbitPredictor(SpaceSystem* system, NewtonBody* owner)
{
	this->system = system;
	this->owner = owner;
}


void OrbitPredictor::update(double dt, double t)
{
	int count = past_points_time / dt;
	this->def_past.push_back(OrbitSnapshot(this->owner->state, t));

	// Remove a point to keep number of points constant
	if (this->def_past.size() >= count)
	{
		this->def_past.pop_front();
	}

	spdlog::get("OSP")->info("Point count: {}", this->def_past.size());

	update_mesh();
}

OrbitPredictor::~OrbitPredictor()
{
}

glm::dvec3 ReferenceFrame::transform(glm::dvec3 inertial, double t)
{
	// Centered means that said body is at the origin
	glm::dvec3 diff = inertial - center->last_state.pos;

	// Apply rotational frame

	// (Diff must later be centered on the center body by the renderer
	// using a shader transform)
	return diff;
}
