#pragma once
#include <vector>
#include "quad_tree_node.h"
#include "../../util/defines.h"

struct PlanetTilePath
{
	enum PlanetSide
	{
		PX, NX, PY, NY, PZ, NZ
	};

	std::vector<QuadTreeNode::QuadTreeQuadrant> path;
	PlanetSide side;

	size_t get_depth();
	glm::dvec2 get_min();
	double getSize();

	glm::vec3 get_tile_postrotation();
	glm::vec3 get_tile_origin();
	glm::vec3 get_tile_rotation();
	glm::vec3 get_tile_translation(bool spheric = false);
	glm::vec3 get_tile_scale();
	glm::vec3 get_tile_postscale();

	glm::mat4 get_model_matrix();
	glm::mat4 get_model_spheric_matrix();



	PlanetTilePath(std::vector<QuadTreeNode::QuadTreeQuadrant> path, PlanetSide side)
	{
		this->path = path;
		this->side = side;
	}
};

bool operator==(const PlanetTilePath& a, const PlanetTilePath& b);

struct PlanetTilePathHasher
{
	std::size_t operator()(const PlanetTilePath &t) const
	{
		std::size_t ret = 0;
		hash_combine(ret, t.side);
		for (size_t i = 0; i < t.path.size(); i++)
		{
			hash_combine(ret, t.path[i]);
		}

		return ret;
	}
};
