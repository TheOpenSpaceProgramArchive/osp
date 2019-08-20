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
		if (!tile->needs_upload)
		{
			glm::mat4 model = tile->path.get_model_spheric_matrix();


			//model = glm::mat4();

			float col = tile->path.getSize() * 2.0f;

			shader->setmat4("model", model);
			shader->setvec4("color", glm::vec4(col, sqrt(col), 0.0f, 1.0f));
			glBindVertexArray(tile->vao);
			glDrawElements(GL_TRIANGLES, tile->indices.size(), GL_UNSIGNED_SHORT, (void*)0);
			glBindVertexArray(0);


			//spdlog::get("OSP")->info("D: {}", tile->vao);

			for (size_t i = 0; i < 4; i++)
			{
				if (tile->needs_lower[i])
				{
					glBindVertexArray(tile->tolower_vao[i]);
					glDrawElements(GL_TRIANGLES, tile->tolower[i].size(), GL_UNSIGNED_SHORT, (void*)0);
					glBindVertexArray(0);
				}
				else
				{
					glBindVertexArray(tile->tosame_vao[i]);
					glDrawElements(GL_TRIANGLES, tile->tosame[i].size(), GL_UNSIGNED_SHORT, (void*)0);
					glBindVertexArray(0);
				}

			}
		}
	}


}


bool QuadTreePlanet::needs_lowq_real(QuadTreeNode* node, QuadTreeNode::QuadTreeSide dir, PlanetTilePath::PlanetSide side)
{
	if (node == NULL)
	{
		return false;
	}

	if (node->needs_lowq(dir))
	{
		return true;
	}
	else
	{
		return false;
		// TODO: We need a way to smooth seams a bit further, but 
		// so far it works decently
	}
}

void QuadTreePlanet::update(float dt)
{

	for (PlanetTile* tile : tiles)
	{
		tile_server.unload(tile->path);
	}

	tiles.clear();

	for (size_t i = 0; i < 6; i++)
	{
		QuadTreeNode* target;
		PlanetTilePath::PlanetSide side;

		if (i == 0)			{ target = &px; side = PlanetTilePath::PX; }
		else if (i == 1)	{ target = &py; side = PlanetTilePath::PY; }
		else if (i == 2)	{ target = &pz; side = PlanetTilePath::PZ; }
		else if (i == 3)	{ target = &px; side = PlanetTilePath::NX; }
		else if (i == 4)	{ target = &ny; side = PlanetTilePath::NY; }
		else 				{ target = &nz; side = PlanetTilePath::NZ; }

		for (QuadTreeNode* leaf : target->get_all_leaf_nodes())
		{
			PlanetTilePath path = PlanetTilePath(leaf->get_path(), side);

			bool u_lowq = needs_lowq_real(leaf, QuadTreeNode::NORTH, side);
			bool r_lowq = needs_lowq_real(leaf, QuadTreeNode::EAST, side);
			bool d_lowq = needs_lowq_real(leaf, QuadTreeNode::SOUTH, side);
			bool l_lowq = needs_lowq_real(leaf, QuadTreeNode::WEST, side);

			PlanetTile* tile = tile_server.load(path, u_lowq, r_lowq, d_lowq, l_lowq);

			if (tile == NULL)
			{
				QuadTreeNode* parent_leaf = leaf->parent;
				PlanetTilePath parent_path = PlanetTilePath(parent_leaf->get_path(), side);

				bool u_lowq = needs_lowq_real(parent_leaf, QuadTreeNode::NORTH, side);
				bool r_lowq = needs_lowq_real(parent_leaf, QuadTreeNode::EAST, side);
				bool d_lowq = needs_lowq_real(parent_leaf, QuadTreeNode::SOUTH, side);
				bool l_lowq = needs_lowq_real(parent_leaf, QuadTreeNode::WEST, side);

				PlanetTile* parent_tile = tile_server.load(parent_path, u_lowq, r_lowq, d_lowq, l_lowq);

				auto it = std::find(tiles.begin(), tiles.end(), parent_tile);

				if (it == tiles.end() && parent_tile != NULL)
				{
					tiles.push_back(parent_tile);
				}
			}
			else
			{
				tiles.push_back(tile);
			}

			
		}
	}

	// Unloading before uploading helps avoid GPU memory spikes
	tile_server.update();
}

void QuadTreePlanet::make_all_leafs_at_least(size_t depth, bool exclude_opposite)
{
	std::vector<QuadTreeNode*> targets;

	if (exclude_opposite)
	{
		if (px.has_children())
		{
			targets.push_back(&px);
		}
		if (py.has_children())
		{
			targets.push_back(&py);
		}
		if (pz.has_children())
		{
			targets.push_back(&pz);
		}
		if (nx.has_children())
		{
			targets.push_back(&nx);
		}
		if (ny.has_children())
		{
			targets.push_back(&ny);
		}
		if (nz.has_children())
		{
			targets.push_back(&nz);
		}
	}
	else
	{
		targets.push_back(&px);
		targets.push_back(&py);
		targets.push_back(&pz);
		targets.push_back(&nx);
		targets.push_back(&ny);
		targets.push_back(&nz);
	}
	

	// We have to do this like this to guarantee that neighbouring
	// sides get the correct neighbour depth

	std::vector<int> counts;
	counts.resize(targets.size(), 0);
	
	bool any = false;
	
	std::vector<bool> finished;
	finished.resize(targets.size(), false);

	do
	{
		any = false;

		for (size_t i = 0; i < targets.size(); i++)
		{
			if (finished[i] == false)
			{
				auto leafs = targets[i]->get_all_leaf_nodes();
				counts[i] = leafs.size();

				for (auto leaf : leafs)
				{
					if (leaf->depth == depth - 1)
					{
						leaf->split(false);
					}
					else if (leaf->depth < depth - 1)
					{
						leaf->split(true);
					}
					else
					{
						counts[i]--;
					}
				}

				if (counts[i] > 0)
				{
					any = true;
				}
				else
				{
					finished[i] = true;
				}
			}
		}

	} while (any == true);



	for (size_t i = 0; i < targets.size(); i++)
	{
		auto leafs = targets[i]->get_all_leaf_nodes();
		for (auto leaf : leafs)
		{
			if (leaf->depth == depth)
			{
				leaf->obtain_neighbors(leaf->quad, true);
			}
		}
	}
}

PlanetTilePath::PlanetSide QuadTreePlanet::get_planet_side(glm::vec3 f)
{
	float xabs = glm::abs(f.x);
	float yabs = glm::abs(f.y);
	float zabs = glm::abs(f.z);

	if (xabs >= yabs && xabs >= zabs)
	{
		return f.x >= 0.0f ? PlanetTilePath::PX : PlanetTilePath::NX;
	}

	if (yabs >= xabs && yabs >= zabs)
	{
		return f.y >= 0.0f ? PlanetTilePath::PY : PlanetTilePath::NY;
	}
	
	if (zabs >= xabs && zabs >= yabs)
	{
		return f.z >= 0.0f ? PlanetTilePath::PZ : PlanetTilePath::NZ;
	}


	return PlanetTilePath::PX;
}

glm::dvec2 QuadTreePlanet::get_planet_side_position(glm::vec3 normalized_pos, PlanetTilePath::PlanetSide side)
{
	glm::dvec3 cube = MathUtil::sphere_to_cube(normalized_pos);

	cube += glm::dvec3(1.0f, 1.0f, 1.0f);
	cube /= 2.0;

	if (isnan(cube.x))
	{
		cube.x = 0.5f;
	}

	if (isnan(cube.y))
	{
		cube.y = 0.5f;
	}

	if (isnan(cube.z))
	{
		cube.z = 0.5f;
	}

	if (side != PlanetTilePath::NX && side != PlanetTilePath::PY && side != PlanetTilePath::NY)
	{
		cube.x = 1.0f - cube.x;
		cube.y = 1.0f - cube.y;
		cube.z = 1.0f - cube.z;
	}
	
	if (side == PlanetTilePath::PZ)
	{
		cube.x = 1.0f - cube.x;
	}

	if (side == PlanetTilePath::NY)
	{
		cube.x = 1.0f - cube.x;
	}

	if (side == PlanetTilePath::PX || side == PlanetTilePath::NX)
	{
		return glm::dvec2(cube.z, cube.y);
	}
	else if (side == PlanetTilePath::PY || side == PlanetTilePath::NY)
	{
		return glm::dvec2(cube.z, cube.x);
	}
	else
	{
		return glm::dvec2(cube.x, cube.y);
	}
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


void QuadTreePlanet::draw_gui_window(glm::dvec2 focusPoint, QuadTreeNode* onNode)
{
	const int SIZE = 200;

	ImGui::Begin("QuadTreePlanet");

	ImGui::Checkbox("Auto-Rebuild", &auto_rebuild);
	ImGui::SameLine();
	if (ImGui::Button("Rebuild Now"))
	{
		tile_server.rebuild_all();
		was_building = true;
	}

	if (planet->surface_provider.dirty)
	{
		if (tile_server.being_worked_on == 0)
		{
			if(was_building)
			{ 
				planet->surface_provider.dirty = false;
			}
			else
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Planet is not updated!");
				ImGui::SameLine();
				if (ImGui::SmallButton("Rebuild") || auto_rebuild == true)
				{
					tile_server.rebuild_all();
					was_building = true;
				}
			}
		}
		else
		{
			ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Planet is being built!");
			was_building = true;
		}
	}
	else
	{
		if (tile_server.being_worked_on == 0)
		{
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Planet is up-to-date");
		}
		else
		{
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.6f, 1.0f), "Planet is up-to-date, but generating");
		}

		was_building = false;
		
	}
	

	if (ImGui::CollapsingHeader("Surface Generation", ImGuiTreeNodeFlags_DefaultOpen))
	{
		planet->surface_provider.draw_imgui();
	}

	if (ImGui::CollapsingHeader("QuadTree sides"))
	{

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

	}


	ImGui::End();
}
