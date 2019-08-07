#pragma once
#include <glm/glm.hpp>
#include "../../orbital/planet.h"

// Base class for an arbitrary surface provider 
// This one provides a totally flat planet
class SurfaceProvider
{
	// Coordinates are given in polar coordinates being:
	//	x -> 
	//	y ->
	// You are expected to return the radius from the planet center
	virtual double getHeight(glm::dvec2 coord, const Planet& planet)
	{
		return planet.radius;
	}


};