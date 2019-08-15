#include "layered_surface_provider.h"



void LayeredSurfaceProvider::get_heights(PlanetTilePath& path, size_t verts, std::vector<float>& out, 
	const Planet& planet, glm::mat4 sphere_model, glm::mat4 cube_model, glm::mat4 path_model)
{
	for (size_t i = 0; i < layers.size(); i++)
	{
		SurfaceProvider* layer = layers[i].provider;
		if (path.get_depth() >= layers[i].min_depth)
		{
			layer->get_heights(path, verts, out, planet, sphere_model, cube_model, path_model);
		}
	}
}

LayeredSurfaceProvider::LayeredSurfaceProvider()
{
}


LayeredSurfaceProvider::~LayeredSurfaceProvider()
{
}
