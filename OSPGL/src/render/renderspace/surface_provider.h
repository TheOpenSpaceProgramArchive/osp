#pragma once
#include <glm/glm.hpp>
#include "../../orbital/planet.h"

// Base class for an arbitrary surface provider 
// This one provides a totally flat planet
class SurfaceProvider
{
public:

	// Coordinates are given in polar coordinates being:
	//	x -> 
	//	y ->
	// You are expected to return the deviation from planet radius in meters
	virtual double getHeight(glm::dvec2 coord, const Planet& planet)
	{
		return 0.0;
	}


};