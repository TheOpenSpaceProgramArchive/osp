#pragma once

#include "space_body.h"


#ifndef G
#define G (6.67 * std::pow(10, -11))
#endif


class SpaceSystem
{
public:

	std::vector<SpaceBody> bodies;

	// Computes instantaneous gravitational force at a given point
	// Mass of second object is assumed to be extremely small compared to the
	// planetary body, but we could implement it too.
	glm::dvec3 computeForce(glm::dvec3 pos);

	SpaceSystem();
	~SpaceSystem();
};

