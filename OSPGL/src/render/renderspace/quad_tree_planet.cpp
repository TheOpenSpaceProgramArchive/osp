#include "quad_tree_planet.h"

bool QuadTreePlanet::can_see(glm::dvec3 dir, glm::dvec3 nrm_cam)
{
	// TODO
	return true;
}

void QuadTreePlanet::update_lod(glm::dvec3 cameraPos, double distance, QuadTreeNode node, glm::dvec3 relative)
{

	// Subdivide until wanted quality level for distance
	size_t maxDepth = 4;

}

QuadTreeCoordinate QuadTreePlanet::project(glm::dvec3 nrm_vec)
{
	QuadTreeCoordinate out;

	// Our mapping guarantees that a ray casted outwards will go through
	// the same QuadTreeNode in the sphere mapping and in the cube mapping
	// so we have to find the side of the cube that is hit

	return out;
}

void QuadTreePlanet::flatten()
{
	px.merge(); nx.merge();
	py.merge(); ny.merge();
	pz.merge(); nz.merge();
}

void QuadTreePlanet::update_lod(glm::dvec3 cameraPos, double distance)
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
	if (!can_see(to_px, normalized_camera)) { px.merge(); } else { update_lod(cameraPos, distance, px, to_px); }
	if (!can_see(to_nx, normalized_camera)) { nx.merge(); } else { update_lod(cameraPos, distance, nx, to_nx); }

	if (!can_see(to_py, normalized_camera)) { py.merge(); } else { update_lod(cameraPos, distance, py, to_py); }
	if (!can_see(to_ny, normalized_camera)) { ny.merge(); } else { update_lod(cameraPos, distance, ny, to_ny); }

	if (!can_see(to_pz, normalized_camera)) { pz.merge(); } else { update_lod(cameraPos, distance, pz, to_pz); }
	if (!can_see(to_nz, normalized_camera)) { nz.merge(); } else { update_lod(cameraPos, distance, nz, to_nz); }
}

void QuadTreePlanet::draw_gui_window(glm::dvec2 focusPoint, QuadTreeNode* onNode)
{
	const int SIZE = 200;

	ImGui::Begin("QuadTreePlanet");

	ImGui::Text("X (P/N)");

	ImGui::BeginChild("PX", ImVec2(SIZE, SIZE));
	px.draw_gui(SIZE - 1, focusPoint, onNode);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("NX", ImVec2(SIZE, SIZE));
	nx.draw_gui(SIZE - 1, focusPoint, onNode);
	ImGui::EndChild();

	ImGui::Text("Y (P/N)");

	ImGui::BeginChild("PY", ImVec2(SIZE, SIZE));
	py.draw_gui(SIZE - 1, focusPoint, onNode);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("NY", ImVec2(SIZE, SIZE));
	ny.draw_gui(SIZE - 1, focusPoint, onNode);
	ImGui::EndChild();

	ImGui::Text("Z (P/N)");

	ImGui::BeginChild("PZ", ImVec2(SIZE, SIZE));
	pz.draw_gui(SIZE - 1, focusPoint, onNode);
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginChild("NZ", ImVec2(SIZE, SIZE));
	nz.draw_gui(SIZE - 1, focusPoint, onNode);
	ImGui::EndChild();

	ImGui::End();
}

QuadTreePlanet::QuadTreePlanet() :
	px(), py(), pz(), nx(), ny(), nz()
{
	// Cardinal directions have lost all meaning, keep that in mind
	// as this is a totally arbitrary combination of neighbors that just
	// satisfies that positive and negative sides are opposite and not connected
	// Maybe naming neighbors with cardinal directions is not a good idea, but it works

	px.neighbors[QuadTreeNode::NORTH] = &py;
	px.neighbors[QuadTreeNode::EAST] = &pz;
	px.neighbors[QuadTreeNode::SOUTH] = &ny;
	px.neighbors[QuadTreeNode::WEST] = &nz;

	nx.neighbors[QuadTreeNode::NORTH] = &pz;
	nx.neighbors[QuadTreeNode::EAST] = &ny;
	nx.neighbors[QuadTreeNode::SOUTH] = &nz;
	nx.neighbors[QuadTreeNode::WEST] = &py;

	py.neighbors[QuadTreeNode::NORTH] = &nz;
	py.neighbors[QuadTreeNode::SOUTH] = &px;
	py.neighbors[QuadTreeNode::EAST] = &nx;
	py.neighbors[QuadTreeNode::WEST] = &pz;

	pz.neighbors[QuadTreeNode::EAST] = &py;
	pz.neighbors[QuadTreeNode::WEST] = &px;
	pz.neighbors[QuadTreeNode::SOUTH] = &nx;
	pz.neighbors[QuadTreeNode::NORTH] = &ny;

	ny.neighbors[QuadTreeNode::NORTH] = &px;
	ny.neighbors[QuadTreeNode::WEST] = &nx;
	ny.neighbors[QuadTreeNode::EAST] = &nz;
	ny.neighbors[QuadTreeNode::SOUTH] = &pz;

	nz.neighbors[QuadTreeNode::EAST] = &px;
	nz.neighbors[QuadTreeNode::NORTH] = &nx;
	nz.neighbors[QuadTreeNode::SOUTH] = &py;
	nz.neighbors[QuadTreeNode::WEST] = &ny;
}