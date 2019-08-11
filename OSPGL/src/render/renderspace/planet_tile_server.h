#pragma once
#include <unordered_map>
#include "../../orbital/planet.h"
#include "quad_tree_node.h"
#include "../../util/defines.h"
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/normal.hpp>
#include "../renderlow/mesh.h"
#include "../../util/math_util.h"
#include <vector>

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

struct PlanetTile
{
	PlanetTilePath path;

	size_t vert_count;
	const Planet& planet;

	std::vector<float> verts;
	std::vector<uint16_t> indices;
	// Up, right, down, left
	std::vector<uint16_t> tolower[4];
	// Up, right, down, left
	std::vector<uint16_t> tosame[4];

	int users;

	GLuint vbo, vao, ebo;
	GLuint tolower_vao[4], tolower_ebo[4];
	GLuint tosame_vao[4], tosame_ebo[4];

	// Up, right, down, left, like always
	bool needs_lower[4];

	bool isUploaded();
	// Any combination of sides may be a lower quality (TODO)
	void upload();
	void unload();

	void generate();

	void generate_vertex(size_t ix, size_t iy, size_t vertCount, size_t vertCountHeight,
		std::vector<float>& heights, glm::mat4 model,
		glm::mat4 inverse_model_spheric, size_t index, std::vector<float>& target, float size);

	void generate_normal(size_t i, std::vector<uint16_t>& indices, std::vector<float>& verts, size_t FLOATS_PER_VERTEX,
		glm::mat4 model_spheric);

	PlanetTile(PlanetTilePath nPath, size_t verticesPerSide, const Planet& planet);
};


// Handles generation and caching of planetary tiles
class PlanetTileServer
{
public:



	Planet* planet;

	std::unordered_map<PlanetTilePath, PlanetTile*, PlanetTilePathHasher> tiles;

	PlanetTile* load(PlanetTilePath path, bool low_up, bool low_right, bool low_down, bool low_left, bool now = false);
	void unload(PlanetTilePath path, bool unload_now = false);

	// Unloads all unused tiles
	void unload_unused();

	// Uploads all used tiles which are not uploaded
	void upload_used();

	// Minimum depth at which tiles get unloaded, bigger tiles
	// will be loaded even if not used
	size_t minDepthToUnload = 2;

	// Vertices in the side of each tile, smallest tiles
	// simply get smallest heightmap samples
	size_t verticesPerSide = 32 + 1;

	PlanetTileServer(Planet* planet);
	~PlanetTileServer();
};

