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

	// Output energy for every newton body
	for (size_t i = 0; i < newton_bodies.size(); i++)
	{

	}
	
}


SpaceSystem::SpaceSystem()
{
}


SpaceSystem::~SpaceSystem()
{
}
