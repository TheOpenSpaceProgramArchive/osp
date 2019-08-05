#pragma once
#include "quad_tree_node.h"
#include <glm/glm.hpp>

class QuadTreePlanet
{
private:
	bool can_see(glm::dvec3 dir, glm::dvec3 nrm_cam);
	void updateLOD(glm::dvec3 cameraPos, double distance, QuadTreeNode node, glm::dvec3 relative);

public:

	QuadTreeNode px, nx, py, ny, pz, nz;

	// Parameters of the ellipsoid
	double ea = 1000.0, eb = 1000.0, ec = 1000.0;


	void updateLOD(glm::dvec3 cameraPos, double distance);

	QuadTreePlanet();
};