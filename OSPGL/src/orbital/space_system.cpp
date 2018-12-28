#include "space_system.h"



glm::dvec3 SpaceSystem::compute_force(glm::dvec3 pos)
{
	glm::dvec3 out;

	for (size_t i = 0; i < bodies.size(); i++)
	{
		glm::dvec3 body_pos = bodies[i]->last_state.pos;
		double dist = glm::distance(pos, body_pos);
		double intensity = bodies[i]->mass / (dist * dist);
		intensity *= G;

		out += glm::normalize(body_pos - pos) * intensity;
	}

	return out;
}

glm::dvec3 SpaceSystem::compute_force_at(glm::dvec3 pos, double t0)
{
	glm::dvec3 out;

	for (size_t i = 0; i < bodies.size(); i++)
	{
		double true_anom = bodies[i]->mean_to_true(bodies[i]->time_to_mean(t0));
		glm::dvec3 body_pos = bodies[i]->to_state_at(true_anom, t0).pos;
		double dist = glm::distance(pos, body_pos);
		double intensity = bodies[i]->mass / (dist * dist);
		intensity *= G;

		out += glm::normalize(body_pos - pos) * intensity;
	}

	return out;
}

// Returns the acceleration experimented by said state
// (aka force)
glm::dvec3 rk4_function(SpaceSystem* sys, NewtonState st, double t, double dt)
{
	return glm::dvec3();
}

RK4Derivative rk4_compute(SpaceSystem* sys, NewtonState initial, double t, double dt, RK4Derivative der)
{
	NewtonState st;
	st.pos = initial.pos + der.pos * dt;
	st.delta = initial.delta + der.delta * dt;

	RK4Derivative out;
	out.pos = st.delta;
	out.delta = rk4_function(sys, st, t, dt);
	return out;
}


void SpaceSystem::simulate(float timewarp, float dt, double* t, NewtonBody::SolverMethod method)
{
	int loops = timewarp / dt;
	double realDelta = dt;

	for (size_t i = 0; i < loops; i++)
	{

		for (size_t j = 0; j < bodies.size(); j++)
		{
			bodies[j]->true_anomaly = bodies[j]->mean_to_true(bodies[j]->time_to_mean(time));
			bodies[j]->last_state = bodies[j]->to_state(time);
		}

		for (size_t j = 0; j < newton_bodies.size(); j++)
		{

			if (method == NewtonBody::SolverMethod::EULER)
			{
				NewtonState copy = newton_bodies[j]->state;
				glm::dvec3 force = compute_force(copy.pos) * realDelta;
				copy.add_force(force);
				copy.pos += copy.delta * realDelta;
				newton_bodies[j]->state = copy;
			}
			else if (method == NewtonBody::SolverMethod::VERLET)
			{
				// !!This is actually velocity verlet!!

				NewtonState copy = newton_bodies[j]->state;
				glm::dvec3 force = compute_force(copy.pos);
				//glm::dvec3 pos = copy.pos * 2.0;
				//pos -= copy.prev;
				//pos += force * realDelta;

				glm::dvec3 pos = copy.pos;
				glm::dvec3 delta = copy.delta;
				pos += copy.delta * realDelta + 0.5 * force * realDelta * realDelta;
				delta += force * realDelta;

				newton_bodies[j]->state.pos = pos;
				newton_bodies[j]->state.delta = delta;
				newton_bodies[j]->state.prev = copy.pos;
			}
			else if (method == NewtonBody::SolverMethod::RUNGE_KUTTA)
			{
				//spdlog::get("OSP")->error("NOT IMPLEMENTED RUNGE KUTTA IN USE!");
				// TODO
				NewtonState copy = newton_bodies[j]->state;
				RK4Derivative d0, d1, d2, d3;

				d0 = rk4_compute(this, copy, time, 0.0, RK4Derivative());
				d1 = rk4_compute(this, copy, time, realDelta * 0.5, d0);
				d2 = rk4_compute(this, copy, time, realDelta * 0.5, d1);
				d3 = rk4_compute(this, copy, time, realDelta, d2);

			
			}
		}

		*t += realDelta;
		time += realDelta;
	}
	
}

NewtonState SpaceSystem::simulate_static(double dt, double t0, NewtonState st)
{
	NewtonState copy = st;
	glm::dvec3 force = compute_force(copy.pos) * dt;
	copy.add_force(force);
	copy.pos += copy.delta * dt;

	st.pos = copy.pos;
	st.delta = copy.delta;

	return st;
}

static std::string serialize_body(SpaceBody* body)
{
	std::string out;

	out += "B";

	// Data order:
	// mass, eccentricity, smajor_axis, inclination, asc_node, arg_periapsis, true_anomaly

	out += body->id; out += ";";
	if (body->parent == NULL)
	{
		out += "NULL"; out += ";";
	}
	else
	{
		out += body->parent->id; out += ";";
	}
	
	out += std::to_string(body->mass); out += ";";
	out += std::to_string(body->smajor_axis); out += ";";
	out += std::to_string(body->inclination); out += ";";
	out += std::to_string(body->asc_node); out += ";";
	out += std::to_string(body->arg_periapsis); out += ";";
	out += std::to_string(body->eccentricity); out += ";";
	out += std::to_string(body->true_anomaly); out += ";";


	out += "\n";

	return out;
}

static std::string serialize_newton(NewtonBody* body)
{
	std::string out;

	// Data order: pos, delta (if we go back to verlet, prev and dt)
	NewtonState state = body->state;

	out += "N";

	out += std::to_string(state.pos.x); out += ",";
	out += std::to_string(state.pos.y); out += ",";
	out += std::to_string(state.pos.z); out += ";";

	out += std::to_string(state.delta.x); out += ",";
	out += std::to_string(state.delta.y); out += ",";
	out += std::to_string(state.delta.z); out += ";";

	out += "\n";
	return out;
}

// Throws 0 if it's an error value
static double deserialize_double(std::string data)
{
	std::istringstream i(data);
	double x;
	if (!(i >> x))
	{
		throw(0);
	}
	return x;
}

// Throws 0 if it's an error value
static glm::dvec3 deserialize_vec(std::string vec)
{
	std::vector<std::string> nums;
	std::string buffer;
	for (size_t i = 0; i < vec.size(); i++)
	{
		if (vec[i] == ',')
		{
			nums.push_back(buffer);
			buffer.clear();
		}
		else
		{
			buffer.push_back(vec[i]);
		}
	}

	// Push last number
	nums.push_back(buffer);
	buffer.clear();

	if (nums.size() != 3)
	{
		throw(0);
	}
	
	double x = deserialize_double(nums[0]);
	double y = deserialize_double(nums[1]);
	double z = deserialize_double(nums[2]);

	return glm::dvec3(x, y, z);
}



static std::vector<std::string> break_chunk(std::string chunk)
{
	std::vector<std::string> out;
	std::string buffer;

	for (size_t i = 0; i < chunk.size(); i++)
	{
		if (chunk[i] == ';')
		{
			out.push_back(buffer);
			buffer.clear();
		}
		else if (i >= 1)
		{
			buffer.push_back(chunk[i]);
		}
	}

	return out;
}

static void deserialize_chunk(std::string chunk, SpaceSystem* sys, int line)
{
	if (chunk[0] == 'B')
	{
		// Kepler Body
		std::vector<std::string> data = break_chunk(chunk);

		if (data.size() != 9)
		{
			spdlog::get("OSP")->warn("[Deserialize Error] (line: {}): Invalid kepler body datasize", line);
			return;
		}

		SpaceBody* body = new SpaceBody();
		double mass, smajor_axis, inclination, asc_node, arg_periapsis, eccentricity, true_anomaly;
		try
		{
			mass = deserialize_double(data[2]);
			smajor_axis = deserialize_double(data[3]);
			inclination = deserialize_double(data[4]);
			asc_node = deserialize_double(data[5]);
			arg_periapsis = deserialize_double(data[6]);
			eccentricity = deserialize_double(data[7]);
			true_anomaly = deserialize_double(data[8]);
		}
		catch (int)
		{
			spdlog::get("OSP")->warn("[Deserialize Error] (line: {}): Malformed kepler data", line);
		}

		body->id = data[0];
		body->parent_id = data[1];
		body->mass = mass;
		body->smajor_axis = smajor_axis;
		body->inclination = inclination;
		body->asc_node = asc_node;
		body->arg_periapsis = arg_periapsis;
		body->eccentricity = eccentricity;
		body->true_anomaly = true_anomaly;

		sys->bodies.push_back(body);
	}
	else if (chunk[0] == 'N')
	{
		// Newton Body (Vessel)
		std::vector<std::string> data = break_chunk(chunk);

		if (data.size() != 2)
		{
			spdlog::get("OSP")->warn("[Deserialize Error] (line: {}): Invalid newton body datasize", line);
			return;
		}
		
		glm::dvec3 arg0v, arg1v;

		try
		{
			arg0v = deserialize_vec(data[0]);
			arg1v = deserialize_vec(data[1]);
		}
		catch(int)
		{
			spdlog::get("OSP")->warn("[Deserialize Error] (line: {}): Malformed newton vector data", line);
			return;
		}

		NewtonBody* body = new NewtonBody();
		body->state.pos = arg0v;
		body->state.delta = arg1v;
		body->state.dir = glm::normalize(body->state.delta);
		body->state.vel = glm::length(body->state.delta);

		sys->newton_bodies.push_back(body);
	}
	else
	{
		spdlog::get("OSP")->warn("[Deserialize Error] (line: {}): Invalid chunk identifier", line);
	}
}

void SpaceSystem::deserialize(std::string data)
{
	spdlog::get("OSP")->info("Starting to deserialize");
	// Break into "chunks", groups of data
	std::string buffer = "";
	int line = 0;
	for (size_t i = 0; i < data.size(); i++)
	{
		if (data[i] == '\n')
		{
			deserialize_chunk(buffer, this, line);
			buffer.clear();
			line++;
		}
		else
		{
			buffer += data[i];
		}
	}

	for (size_t i = 0; i < bodies.size(); i++)
	{
		std::string to_find = bodies[i]->parent_id;
		for (size_t j = 0; j < bodies.size(); j++)
		{
			if (bodies[j]->id == to_find)
			{
				bodies[i]->parent = bodies[j];
			}
		}
	}
}

void SpaceSystem::draw_debug_data(DebugDraw* debug, double scale)
{
	for (size_t i = 0; i < bodies.size(); i++)
	{
		debug->add_cross(bodies[i]->last_state.pos * scale, 0.1f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	for (size_t i = 0; i < newton_bodies.size(); i++)
	{
		debug->add_cross(newton_bodies[i]->state.pos * scale, 0.05f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		//debug->add_point(newton_bodies[i]->state.pos * scale, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 3.0f, 1.0f);
	}
}

std::string SpaceSystem::serialize()
{
	std::string out;

	for (size_t i = 0; i < bodies.size(); i++)
	{
		out += serialize_body(bodies[i]);
	}

	for (size_t i = 0; i < newton_bodies.size(); i++)
	{
		out += serialize_newton(newton_bodies[i]);
	}

	return out;
}

SpaceSystem::SpaceSystem()
{
	plot = MultiPlot();
	plot.create_plot("energy", glm::vec3(1.0, 0.0, 0.0));
	plot.create_plot("kinetic", glm::vec3(0.0, 1.0, 0.0));
	plot.create_plot("potential", glm::vec3(0.4, 0.4, 1.0));
}


SpaceSystem::~SpaceSystem()
{
}
