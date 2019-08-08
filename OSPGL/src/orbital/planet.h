#pragma once
// Holds structural data for a planet
// Note: It will not deallocate the surface provider, which can be NULL

class SurfaceProvider;

struct Planet
{
	// Radius in meters 
	double radius;

	SurfaceProvider* surface_provider;

	Planet()
	{
		surface_provider = NULL;
	}
};