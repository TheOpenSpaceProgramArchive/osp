#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "../util/defines.h"

/*
	A body simulated using newtonian dynamics
	such as vessels.

	Units are as usual the SI units (s, m, etc...)

*/
class newton_body
{
public:

	glm::dvec3 pos;
	glm::dvec3 vel;

	// Steps the simulation
	// Leave subsamples at <= 0 to get automatic
	// subsample calculation
	// Note: dt means the physic step, not framerate!
	void step(double dt, int samples = -1);

	// Calculates a single step of the simulation
	void substep(double dt);

	newton_body();
	~newton_body();
};

