#pragma once
#include "quad_tree_node.h"
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <imgui/imgui.h>
#include "../../orbital/planet.h"
#include "planet_tile_server.h"
#include "../renderlow/shader.h"
#include "../../util/math_util.h"

struct QuadTreeCoordinate
{
	QuadTreeNode* node;
	glm::dvec2 coord;
};

class QuadTreePlanet
{
private:
	bool can_see(glm::dvec3 dir, glm::dvec3 nrm_cam);
	void update_lod(glm::dvec3 cameraPos, double distance, QuadTreeNode node, glm::dvec3 relative);

	// Projects a 3D normalized coordinate to the appropiate
	// coordinate of the appropiate QuadTreeNode
	QuadTreeCoordinate project(glm::dvec3 nrm_vec);

	bool auto_rebuild = true;
	bool was_building = false;

public:

	Shader* shader;

	Planet* planet;

	PlanetTileServer tile_server;

	QuadTreeNode px, nx, py, ny, pz, nz;

	std::vector<PlanetTile*> tiles;

	bool needs_lowq_real(QuadTreeNode* node, QuadTreeNode::QuadTreeSide dir, PlanetTilePath::PlanetSide side);

	void flatten();

	void update_lod(glm::dvec3 cameraPos, double distance);

	void draw_gui_window(glm::dvec2 focusPoint, QuadTreeNode* onNode);

	void draw(glm::mat4 view, glm::mat4 proj);
	void update(float dt);

	// Makes every leaf be (atleast) of given depth.
	// It can optionally exclude the opposite side of the planet
	// (side with depth = 0), as it's not visible under normal conditions
	void make_all_leafs_at_least(size_t depth, bool exclude_opposite = true);

	PlanetTilePath::PlanetSide get_planet_side(glm::vec3 normalized_pos);
	glm::dvec2 get_planet_side_position(glm::vec3 normalized_pos, PlanetTilePath::PlanetSide side);

	QuadTreePlanet(Planet* planet, Shader* shader = NULL);
};