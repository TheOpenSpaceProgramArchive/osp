#pragma once
#include "quad_tree_node.h"
#include <glm/glm.hpp>
#include <imgui/imgui.h>

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

public:

	QuadTreeNode px, nx, py, ny, pz, nz;

	// Parameters of the ellipsoid
	double ea = 1000.0, eb = 1000.0, ec = 1000.0;

	void flatten();

	void update_lod(glm::dvec3 cameraPos, double distance);

	void draw_gui_window(glm::dvec2 focusPoint, QuadTreeNode* onNode);

	QuadTreePlanet();
};