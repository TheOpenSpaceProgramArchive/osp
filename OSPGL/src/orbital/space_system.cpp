#include "space_system.h"



glm::dvec3 SpaceSystem::computeForce(glm::dvec3 pos)
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

RK4Derivative rk4_compute(SpaceSystem* sys, glm::dvec3 cur_pos, glm::dvec3 cur_vel, double dt, RK4Derivative der)
{
	RK4Derivative state = der * dt + RK4Derivative(cur_pos, cur_vel);
	return RK4Derivative(state.vel, sys->computeForce(state.pos));
}


void SpaceSystem::simulate(float timewarp, float dt, float* t, NewtonBody::SolverMethod method)
{
	int loops = timewarp / dt;
	double realDelta = dt;

	for (size_t i = 0; i < loops; i++)
	{

		for (size_t j = 0; j < bodies.size(); j++)
		{
			bodies[j]->true_anomaly = bodies[j]->mean_to_true(bodies[j]->time_to_mean(time));
			bodies[j]->last_state = bodies[j]->to_state();
		}

		for (size_t j = 0; j < newton_bodies.size(); j++)
		{

			if (method == NewtonBody::SolverMethod::EULER)
			{
				NewtonState copy = newton_bodies[j]->state;
				glm::dvec3 force = computeForce(copy.pos) * realDelta;
				copy.add_force(force);
				copy.pos += copy.delta * realDelta;
				newton_bodies[j]->state = copy;
			}
			else if (method == NewtonBody::SolverMethod::VERLET)
			{
				NewtonState copy = newton_bodies[j]->state;
				glm::dvec3 force = computeForce(copy.pos) * realDelta;
				glm::dvec3 pos = copy.pos * 2.0;
				pos -= copy.prev;
				pos += force;

				newton_bodies[j]->state.pos = pos;
				newton_bodies[j]->state.prev = copy.pos;
			}
			else if (method == NewtonBody::SolverMethod::RUNGE_KUTTA)
			{
				spdlog::get("OSP")->error("NOT IMPLEMENTED RUNGE KUTTA IN USE!");
				// TODO
				NewtonState copy = newton_bodies[j]->state;
				RK4Derivative d0 = RK4Derivative();
				RK4Derivative d1 = rk4_compute(this, copy.pos, copy.delta, realDelta * 0.0, d0);
				RK4Derivative d2 = rk4_compute(this, copy.pos, copy.delta, realDelta * 0.5, d1);
				RK4Derivative d3 = rk4_compute(this, copy.pos, copy.delta, realDelta * 0.5, d2);
				RK4Derivative d4 = rk4_compute(this, copy.pos, copy.delta, realDelta * 1.0, d3);

				d3 *= 2;
				d2 += d3;
				d2 *= 1.0 / 6.0;
				d1 += d2;
				d4 += d1;

				copy.pos += d4.pos * realDelta;
				copy.delta += d4.vel * realDelta;
			}
		}

		*t += realDelta;
		time += realDelta;
	}

	// Uncomment to enable energy debugging
	/*ImGui::Begin("Orbital Energy");

	// Output energy for every newton body (assume mass 1)
	for (size_t i = 0; i < newton_bodies.size(); i++)
	{
		ImGui::Text("Body %i", i);

		double potential = 0;
		double vsquared;
		if (method == NewtonBody::SolverMethod::VERLET)
		{
			glm::dvec3 v = newton_bodies[i]->state.pos - newton_bodies[i]->state.prev;
			vsquared = glm::length(v) * glm::length(v);
		}
		else
		{
			vsquared = newton_bodies[i]->state.vel * newton_bodies[i]->state.vel;
		}

		double kinetic = vsquared / 2.0f;

		for (size_t j = 0; j < bodies.size(); j++)
		{
			double dist = glm::distance(newton_bodies[i]->state.pos, bodies[j]->last_state.pos);
			double pot = -G * (bodies[j]->mass / dist);
			potential += pot;
		}
		
		plot.draw(256, 128);

		plot.add_data("energy", kinetic + potential);
		plot.add_data("kinetic", kinetic);
		plot.add_data("potential", potential);
	}

	ImGui::End();*/
	
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
