#pragma once
#include "../render/renderspace/surface_provider.h"

// Holds structural data for a planet
struct Planet
{
	// Radius in meters 
	double radius;

	SurfaceProvider surface_provider;

};