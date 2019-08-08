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






void QuadTreePlanet::draw(glm::mat4 view, glm::mat4 proj)
{
	if (shader == NULL)
	{
		shader = g_shader;
	}

	shader->use();


	shader->setmat4("view", view);
	shader->setmat4("proj", proj);

	for (auto tile : tiles)
	{

		glm::mat4 model = glm::mat4();
	

		glm::mat4 translation_mat = glm::translate(glm::mat4(), tile->path.get_tile_translation(true));
		glm::mat4 scale_mat = glm::scale(glm::mat4(), tile->path.get_tile_scale());
		glm::mat4 origin_mat = glm::translate(model, tile->path.get_tile_origin());
		glm::mat4 rotation_mat = glm::toMat4(glm::quat(tile->path.get_tile_rotation()));

		// The final translation makes the origin of the tile be on the correct corner
		model = translation_mat * rotation_mat * scale_mat * origin_mat;

		//model = glm::mat4();

		float col = tile->path.getSize() * 2.0f;

		shader->setmat4("model", model);
		shader->setvec4("color", glm::vec4(col, sqrt(col), 0.0f, 1.0f));
		glBindVertexArray(tile->vao);
		glDrawElements(GL_TRIANGLES, tile->indices.size(), GL_UNSIGNED_SHORT, (void*)0);
		glBindVertexArray(0);

	}


}

void QuadTreePlanet::update(float dt)
{

	for (PlanetTile* tile : tiles)
	{
		tile_server.unload(tile->path);
	}

	tiles.clear();

	auto px_leafs = px.getAllLeafNodes();
	auto nx_leafs = nx.getAllLeafNodes();
	auto py_leafs = py.getAllLeafNodes();
	auto ny_leafs = ny.getAllLeafNodes();
	auto pz_leafs = pz.getAllLeafNodes();
	auto nz_leafs = nz.getAllLeafNodes();

	for (QuadTreeNode* leaf : px_leafs)
	{
		PlanetTilePath path = PlanetTilePath(leaf->getPath(), PlanetTilePath::PX);
		tiles.push_back(tile_server.load(path));
	}
	for (QuadTreeNode* leaf : nx_leafs)
	{
		PlanetTilePath path = PlanetTilePath(leaf->getPath(), PlanetTilePath::NX);
		tiles.push_back(tile_server.load(path));
	}
	for (QuadTreeNode* leaf : py_leafs)
	{
		PlanetTilePath path = PlanetTilePath(leaf->getPath(), PlanetTilePath::PY);
		tiles.push_back(tile_server.load(path));
	}
	for (QuadTreeNode* leaf : ny_leafs)
	{
		PlanetTilePath path = PlanetTilePath(leaf->getPath(), PlanetTilePath::NY);
		tiles.push_back(tile_server.load(path));
	}
	for (QuadTreeNode* leaf : pz_leafs)
	{
		PlanetTilePath path = PlanetTilePath(leaf->getPath(), PlanetTilePath::PZ);
		tiles.push_back(tile_server.load(path));
	}
	for (QuadTreeNode* leaf : nz_leafs)
	{
		PlanetTilePath path = PlanetTilePath(leaf->getPath(), PlanetTilePath::NZ);
		tiles.push_back(tile_server.load(path));
	}

	tile_server.unload_unused();
}

QuadTreePlanet::QuadTreePlanet(Planet* planet, Shader* shader) :
	px(), py(), pz(), nx(), ny(), nz(), tile_server(planet)
{
	this->planet = planet;
	this->shader = shader;

	// Cardinal directions have lost all meaning, keep that in mind
	// as this is a totally arbitrary combination of neighbors that just
	// satisfies that positive and negative sides are opposite and not connected
	// Maybe naming neighbors with cardinal directions is not a good idea, but it works


	px.neighbors[QuadTreeNode::NORTH] = &py;
	px.neighbors[QuadTreeNode::EAST] = &nz;
	px.neighbors[QuadTreeNode::SOUTH] = &ny;
	px.neighbors[QuadTreeNode::WEST] = &pz;

	nx.neighbors[QuadTreeNode::NORTH] = &ny;
	nx.neighbors[QuadTreeNode::EAST] = &pz;
	nx.neighbors[QuadTreeNode::SOUTH] = &py;
	nx.neighbors[QuadTreeNode::WEST] = &nz;

	py.neighbors[QuadTreeNode::NORTH] = &nx;
	py.neighbors[QuadTreeNode::SOUTH] = &px;
	py.neighbors[QuadTreeNode::EAST] = &pz;
	py.neighbors[QuadTreeNode::WEST] = &nz;

	pz.neighbors[QuadTreeNode::EAST] = &px;
	pz.neighbors[QuadTreeNode::WEST] = &nx;
	pz.neighbors[QuadTreeNode::SOUTH] = &ny;
	pz.neighbors[QuadTreeNode::NORTH] = &py;

	ny.neighbors[QuadTreeNode::NORTH] = &px;
	ny.neighbors[QuadTreeNode::WEST] = &nz;
	ny.neighbors[QuadTreeNode::EAST] = &pz;
	ny.neighbors[QuadTreeNode::SOUTH] = &nx;

	nz.neighbors[QuadTreeNode::EAST] = &nx;
	nz.neighbors[QuadTreeNode::NORTH] = &py;
	nz.neighbors[QuadTreeNode::SOUTH] = &ny;
	nz.neighbors[QuadTreeNode::WEST] = &px;

	px.planetside = QuadTreeNode::PX;
	py.planetside = QuadTreeNode::PY;
	pz.planetside = QuadTreeNode::PZ;
	nx.planetside = QuadTreeNode::NX;
	ny.planetside = QuadTreeNode::NY;
	nz.planetside = QuadTreeNode::NZ;
}