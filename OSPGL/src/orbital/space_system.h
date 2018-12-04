#pragma once

#include "space_body.h"
#include "newton_body.h"
#ifndef G
#define G (6.67 * std::pow(10, -11))
#endif


class SpaceSystem
{
public:

	// Time since epoch in seconds
	double time;

	std::vector<SpaceBody*> bodies;
	std::vector<NewtonBody*> newton_bodies;

	// Computes instantaneous gravitational force at a given point
	// Mass of second object is assumed to be extremely small compared to the
	// planetary body, but we could implement it too.
	glm::dvec3 computeForce(glm::dvec3 pos);

	// timewarp is the ammount of seconds we fit in one real second,
	// dt is the ammount of seconds that passes since last call to simulate
	void simulate(float timewarp, float dt, float* t);

	SpaceSystem();
	~SpaceSystem();
};

