#pragma once
#include "../surface_provider.h"
// A very useful surface provider that allows stacking many
// other surface providers in a given order

struct SurfaceLayer
{
	SurfaceProvider* provider;
	size_t min_depth;
};

class LayeredSurfaceProvider : public SurfaceProvider
{
public:

	std::vector<SurfaceLayer> layers;

	virtual void get_heights(PlanetTilePath& path, size_t verts, std::vector<float>& out, const Planet& planet,
		glm::mat4 sphere_model, glm::mat4 cube_model, glm::mat4 path_model);


	LayeredSurfaceProvider();
	~LayeredSurfaceProvider();
};

