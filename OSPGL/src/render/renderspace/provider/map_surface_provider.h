#pragma once
#include "../surface_provider.h"
#include "../../renderlow/map.h"

class MapSurfaceProvider : public SurfaceProvider
{
public:

	Map* map;

	virtual void get_heights(PlanetTilePath& path, size_t verts, std::vector<float>& out, const Planet& planet,
		glm::mat4 sphere_model, glm::mat4 cube_model, glm::mat4 path_model);


	MapSurfaceProvider(Map* map);
	~MapSurfaceProvider();
};

