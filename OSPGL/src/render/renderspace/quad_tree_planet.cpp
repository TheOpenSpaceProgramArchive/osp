#include "quad_tree_planet.h"

bool QuadTreePlanet::can_see(glm::dvec3 dir, glm::dvec3 nrm_cam)
{
	// TODO
	return true;
}

void QuadTreePlanet::updateLOD(glm::dvec3 cameraPos, double distance, QuadTreeNode node, glm::dvec3 relative)
{
	// Subdivide until wanted quality level for distance
	size_t maxDepth = 4;

}

void QuadTreePlanet::updateLOD(glm::dvec3 cameraPos, double distance)
{
	// Remove quadtrees that cannot be seen
	glm::dvec3 to_px = glm::dvec3(1.0, 0.0, 0.0);
	glm::dvec3 to_nx = -to_px;
	glm::dvec3 to_py = glm::dvec3(0.0, 1.0, 0.0);
	glm::dvec3 to_ny = -to_py;
	glm::dvec3 to_pz = glm::dvec3(0.0, 0.0, 1.0);
	glm::dvec3 to_nz = -to_pz;

	glm::dvec3 normalized_camera = glm::normalize(cameraPos);

	// We render only the lowest quality if a quadrant can't be seen
	if (!can_see(to_px, normalized_camera)) { px.merge(); } else { updateLOD(cameraPos, distance, px, to_px); }
	if (!can_see(to_nx, normalized_camera)) { nx.merge(); } else { updateLOD(cameraPos, distance, nx, to_nx); }

	if (!can_see(to_py, normalized_camera)) { py.merge(); } else { updateLOD(cameraPos, distance, py, to_py); }
	if (!can_see(to_ny, normalized_camera)) { ny.merge(); } else { updateLOD(cameraPos, distance, ny, to_ny); }

	if (!can_see(to_pz, normalized_camera)) { pz.merge(); } else { updateLOD(cameraPos, distance, pz, to_pz); }
	if (!can_see(to_nz, normalized_camera)) { nz.merge(); } else { updateLOD(cameraPos, distance, nz, to_nz); }
}

QuadTreePlanet::QuadTreePlanet() : px(), nx(), py(), ny(), pz(), nz()
{
	
}