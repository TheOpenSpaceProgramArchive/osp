#include "space_system.h"



glm::dvec3 SpaceSystem::computeForce(glm::dvec3 pos)
{
	glm::dvec3 out;

	for (size_t i = 0; i < bodies.size(); i++)
	{
		glm::dvec3 body_pos = bodies[i]->to_state().pos;
		double dist = glm::distance(pos, body_pos);
		double intensity = bodies[i]->mass / (dist * dist);
		intensity *= G;

		out += glm::normalize(body_pos - pos) * intensity;
	}

	return out;
}

void SpaceSystem::simulate(float timewarp, float dt, float* t)
{
	int loops = timewarp / dt;
	double realDelta = dt;

	for (size_t i = 0; i < loops; i++)
	{
		for (size_t j = 0; j < bodies.size(); j++)
		{
			bodies[j]->true_anomaly = bodies[j]->mean_to_true(bodies[j]->time_to_mean(time));
		}

		for (size_t j = 0; j < newton_bodies.size(); j++)
		{
			NewtonState copy = newton_bodies[j]->state;
			glm::dvec3 force = computeForce(copy.pos) * realDelta;
			copy.add_force(force);
			copy.pos += copy.delta * realDelta;
			newton_bodies[j]->state = copy;
		}

		*t += realDelta;
		time += realDelta;
	}
	
}


SpaceSystem::SpaceSystem()
{
}


SpaceSystem::~SpaceSystem()
{
}
