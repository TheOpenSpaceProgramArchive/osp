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

SpaceSystem::SpaceSystem()
{
}


SpaceSystem::~SpaceSystem()
{
}
