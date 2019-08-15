#include "map_surface_provider.h"



void MapSurfaceProvider::get_heights(PlanetTilePath & path, size_t verts, std::vector<float>& out, const Planet & planet,
	glm::mat4 sphere_model, glm::mat4 cube_model, glm::mat4 path_model)
{

}

MapSurfaceProvider::MapSurfaceProvider(Map* map)
{
	this->map = map;
}


MapSurfaceProvider::~MapSurfaceProvider()
{
}
