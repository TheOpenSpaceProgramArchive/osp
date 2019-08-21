#include "surface_provider.h"
#include <imnodes/imnodes.h>
#include "nodes/all_nodes.h"


void SurfaceProvider::draw_imgui()
{
	bool preview_clicked = false;

	ImGui::Columns(3);

	if (ImGui::Button("New"))
	{
		if (unsaved_changes)
		{
			ImGui::OpenPopup("Confirm Action ##new");
		}
		else
		{
			clear();
		}
	}

	if (ImGui::BeginPopupModal("Confirm Action ##new", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Creating a new graph will delete any unsaved changes, are you sure?");
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Confirm"))
		{
			clear();
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Load"))
	{
		ImGui::OpenPopup("Load File");
		selected_file = "";
	}

	if (ImGui::BeginPopupModal("Load File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (unsaved_changes)
		{
			ImGui::TextColored(ImVec4(1.0, 0.5, 0.5, 1.0), "Unsaved Changes!");
		}

		ImGui::Text("Select a file to load (from res/surfaces)");

		std::vector<std::string> files = FileUtil::get_all_files("res/surfaces/", true);

		if (ImGui::BeginCombo("", selected_file.c_str()))
		{
			for (size_t i = 0; i < files.size(); i++)
			{
				if (ImGui::Selectable(files[i].c_str()))
				{
					selected_file = files[i];
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Confirm"))
		{

			std::string file = FileUtil::load_file(selected_file);
			if (file != "")
			{
				json as_json = json::parse(file);

				clear();

				save_path = selected_file;
				deserialize(as_json);
			}
			

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::SameLine();

	if (ImGui::Button("Save"))
	{
		if (save_path == "")
		{
			ImGui::OpenPopup("Save As");
		}
		else
		{
			save();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Save As"))
	{
		ImGui::OpenPopup("Save As");
	}

	if (ImGui::BeginPopupModal("Save As", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Path: ");

		ImGui::Text("res/surfaces/");
		ImGui::SameLine();
		ImGui::InputText("", path_buf, 512);
		ImGui::SameLine();
		ImGui::Text(".json");

		if (ImGui::Button("Cancel"))
		{
			strcpy(path_buf, "");
			ImGui::CloseCurrentPopup();
		}

		ImGui::SameLine();

		if (ImGui::Button("Confirm"))
		{
			std::string str = std::string(path_buf);
			save_path = "res/surfaces/" + str + ".json";

			save();

			strcpy(path_buf, "");

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	ImGui::NextColumn();

	if (complete)
	{
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Complete!");
	}
	else
	{
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Nodes not satisfied");
	}

	ImGui::NextColumn();

	if (ImGui::Button("Run Previews"))
	{
		preview_clicked = true;
	}
	ImGui::SameLine();
	ImGui::Checkbox("Auto-Preview", &auto_preview);

	ImGui::Columns(1);

	//mtx.lock();
	imnodes::BeginNodeEditor();

	for (auto node : nodes)
	{
		if (node.second == NULL)
		{
			continue;
		}

		imnodes::BeginNode(node.first);
		imnodes::Name(node.second->get_name().c_str());

		for (auto attr : node.second->in_attribute)
		{
			imnodes::BeginInputAttribute(attr.second->id);
			std::string name_str = attr.second->name;
			name_str += " (";
			name_str += SurfaceProviderAttribute::valtype_to_str(attr.second->val_type);
			name_str += ")";
			ImGui::Text("%s", name_str.c_str());
			if (node.second->do_imgui(attr.first))
			{
				dirty = true;
			}
			
			imnodes::EndAttribute();
		}

		for (auto attr : node.second->out_attribute)
		{
			imnodes::BeginOutputAttribute(attr.second->id);
			std::string name_str = attr.second->name;
			name_str += " (";
			name_str += SurfaceProviderAttribute::valtype_to_str(attr.second->val_type);
			name_str += ")";
			ImGui::Text("%s", name_str.c_str());
			if (node.second->do_imgui(attr.first))
			{
				dirty = true;
			}
			imnodes::EndAttribute();
		}

		imnodes::EndNode();
	}

	// Draw links
	for (size_t i = 0; i < links.size(); i++)
	{
		// Color different types, and show type mismatches
		SurfaceProviderAttribute* first = attributes[links[i].first];
		SurfaceProviderAttribute* second = attributes[links[i].second];

		if (first->val_type != second->val_type)
		{
			imnodes::PushColorStyle(imnodes::ColorStyle::ColorStyle_Link, ImColor(1.0f, 0.0f, 0.0f));
		}
		else if(first->val_type == V1)
		{
			imnodes::PushColorStyle(imnodes::ColorStyle::ColorStyle_Link, ImColor(0.8f, 0.8f, 0.8f));
		}
		else if (first->val_type == V2)
		{
			imnodes::PushColorStyle(imnodes::ColorStyle::ColorStyle_Link, ImColor(0.4f, 0.4f, 0.8f));
		}
		else if (first->val_type == V3)
		{
			imnodes::PushColorStyle(imnodes::ColorStyle::ColorStyle_Link, ImColor(0.8f, 0.4f, 0.2f));
		}

		imnodes::Link(i, links[i].first, links[i].second);

		imnodes::PopColorStyle();
	}

	if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() &&
		ImGui::IsMouseClicked(1))
	{
		ImGui::OpenPopup("Right Click Menu");
	}

	if (ImGui::BeginPopup("Right Click Menu"))
	{
		ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

		std::string node_type_str = "Select Node Type";
		if (selected_node_type != "")
		{
			node_type_str = selected_node_type;
		}

		std::vector<std::string> types = get_all_idnames();
		for (const std::string& type : types)
		{
			if (ImGui::MenuItem(type.c_str()))
			{
				selected_node_type = type;
			}
		}

		if (selected_node_type != "")
		{
			SurfaceProviderNode* new_node = create_new_node(selected_node_type, this);
			nodes[new_node->id] = new_node;
			selected_node_type = "";

			imnodes::SetNodePos(new_node->id, click_pos);
		}

		ImGui::EndPopup();
	}

	imnodes::EndNodeEditor();

	int start_attr, end_attr;
	if (imnodes::IsLinkCreated(&start_attr, &end_attr))
	{
		// Validate
		// A link has to start from an output and go to an input
		// A link cannot go to the node it originates from
		if (attributes[end_attr]->is_input && !attributes[start_attr]->is_input 
			&& attributes[end_attr]->owner_id != attributes[start_attr]->owner_id)
		{
			int to_remove = -1;
			// If target attribute is already linked, remove said link
			for (int i = 0; i < links.size(); i++)
			{
				if (links[i].second == end_attr)
				{
					to_remove = i;
				}
			}

			if (to_remove >= 0)
			{
				// We have to remove the link from the output, too

				auto link = links[to_remove];
				attributes[link.first]->remove_link(link.second);
				attributes[link.second]->remove_link(link.first);

				links.erase(links.begin() + to_remove);
			}

			// Push link
			links.push_back(std::make_pair(start_attr, end_attr));
			attributes[start_attr]->links.push_back(end_attr);
			attributes[end_attr]->links.push_back(start_attr);

			dirty = true;
		}
	}

	int link_index;
	int link_sel_index;
	// We need to select a link, and then right click to delete it
	if (imnodes::IsLinkHovered(&link_index) && 
		(ImGui::IsMouseDown(1) || ImGui::IsKeyPressed(ImGuiKey_Delete)) &&
		imnodes::IsLinkSelected(&link_sel_index) && link_index == link_sel_index)
	{
		auto link = links[link_index];
		attributes[link.first]->remove_link(link.second);
		attributes[link.second]->remove_link(link.first);

		links.erase(links.begin() + link_index);

		dirty = true;
	}



	int node_id = -1;
	int hover_id = -1;
	if (imnodes::IsNodeSelected(&node_id) && imnodes::IsNodeHovered(&hover_id) && hover_id == node_id)
	{
		selected_id = node_id;
	}
	else
	{
		selected_id = -1;
	}

	if (ImGui::IsMouseDown(1) && selected_id > 16)
	{
		SurfaceProviderNode* node = nodes[selected_id];
		if (node != NULL)
		{
			for (auto it = links.begin(); it != links.end();)
			{
				bool found = false;

				for (auto inlet : node->in_attribute)
				{
					if (it->first == inlet.second->id || it->second == inlet.second->id)
					{
						found = true;
						break;
					}
				}

				for (auto outlet : node->out_attribute)
				{
					if (it->first == outlet.second->id || it->second == outlet.second->id)
					{
						found = true;
						break;
					}
				}

				if (found)
				{
					it = links.erase(it);
				}
				else
				{
					it++;
				}
			}


			if (node != NULL)
			{

				// Remove links TO this node
				for (auto in_attribute : node->in_attribute)
				{
					if (in_attribute.second->links.size() == 1)
					{
						SurfaceProviderAttribute* other = attributes[in_attribute.second->links[0]];
						int index = -1;
						for (int i = 0; i < other->links.size(); i++)
						{
							if (other->links[i] == in_attribute.second->id)
							{
								index = i;
								break;
							}
						}

						other->links.erase(other->links.begin() + index);
					}
				}

				// Remove links FROM this node
				for (auto out_attribute : node->out_attribute)
				{
					for (size_t i = 0; i < out_attribute.second->links.size(); i++)
					{
						int link_id = out_attribute.second->links[i];
						SurfaceProviderAttribute* other = attributes[link_id];
						other->links.clear();
					}
				}

				SurfaceProviderNode* node = nodes[selected_id];
				nodes.erase(selected_id);
				delete node;

				dirty = true;
			}
		}
	}

	if (dirty && auto_preview || preview_clicked)
	{
		// Find all preview nodes and run preview on them
		for (auto node : nodes)
		{
			PreviewSPN* as_preview = dynamic_cast<PreviewSPN*>(node.second);
			if (as_preview != NULL)
			{
				do_preview(as_preview, planetRadius);
			}
		}
	}

	if (dirty)
	{
		unsaved_changes = true;
	}

	//mtx.unlock();
}

json SurfaceProvider::serialize(bool save_imnodes)
{
	mtx.lock();
	
	json j;

	for (auto node : nodes)
	{
		if (node.second != NULL && node.second->id > 16)
		{
			json imnode_data;

			std::string id = std::to_string(node.first);

			j["nodes"][id]["type"] = node.second->get_name();
			j["nodes"][id]["data"] = node.second->serialize();
		}
	}

	j["links"] = json::array();

	for (auto link : links)
	{
		json jlink;
		
		SurfaceProviderAttribute* a = attributes[link.first];
		SurfaceProviderAttribute* b = attributes[link.second];

		SurfaceProviderNode* a_owner = nodes[a->owner_id];
		SurfaceProviderNode* b_owner = nodes[b->owner_id];

		int a_index = a_owner->find_attribute(a);
		int b_index = b_owner->find_attribute(b);

		jlink["a"]["n"] = a_owner->id;
		jlink["b"]["n"] = b_owner->id;
		jlink["a"]["i"] = a_index;
		jlink["b"]["i"] = b_index;

		j["links"].push_back(jlink);

	}

	if (save_imnodes)
	{
		size_t size;
		const char* data = imnodes::SaveCurrentEditorStateToMemory(&size);

		j["imnodes"] = std::string(data);
	}
	else
	{
		j["imnodes"] = "";
	}

	return j;

	mtx.unlock();
}

void SurfaceProvider::deserialize(json j, bool load_imnodes)
{
	try
	{
		mtx.lock();


		int max_id = -1;
		for (auto jnode : j["nodes"].items())
		{
			int index = std::stoi(jnode.key());
			std::string type = jnode.value()["type"];
			if (index > max_id)
			{
				max_id = index;
			}

			nodes[index] = create_new_node(type, this, index);
		}

		// Creates nodes, so they create their attributes
		for (auto node : nodes)
		{
			if (node.second != NULL)
			{
				node.second->create(this);
			}
		}

		// Give nodes their data
		for (auto node : nodes)
		{
			if (node.second != NULL)
			{
				json j_data = j["nodes"][std::to_string(node.first)]["data"];
				node.second->deserialize(j_data);
			}
		}

		// Read links
		for (auto link : j["links"])
		{
			int start_node_idx = link["a"]["n"];
			int start_attr_idx = link["a"]["i"];
			int end_node_idx = link["b"]["n"];
			int end_attr_idx = link["b"]["i"];

			SurfaceProviderNode* a_node = nodes[start_node_idx];
			SurfaceProviderNode* b_node = nodes[end_node_idx];

			SurfaceProviderAttribute* a_attr = a_node->find_attribute_by_id(start_attr_idx);
			SurfaceProviderAttribute* b_attr = b_node->find_attribute_by_id(end_attr_idx);

			links.push_back(std::make_pair(a_attr->id, b_attr->id));

			// Actually add the link to the attributes
			a_attr->links.push_back(b_attr->id);
			b_attr->links.push_back(a_attr->id);
		}

		imnodes::Clear();

		// Read imnodes data
		if (load_imnodes)
		{
			std::string imnodes = j["imnodes"];
			imnodes::LoadCurrentEditorStateFromMemory(imnodes.c_str(), imnodes.size());
		}

		dirty = true;

		mtx.unlock();
	}
	catch (std::system_error err)
	{
		spdlog::get("OSP")->error(err.what());
	}
}

void SurfaceProvider::save()
{
	json serialized = serialize();

	std::string serialized_str = serialized.dump(4);

	FileUtil::write_file(save_path, serialized_str);
}

SurfaceProviderAttribute* SurfaceProvider::create_attribute(std::string name, int owner_id, bool input, ValueType val_type)
{

	SurfaceProviderAttribute* attr = new SurfaceProviderAttribute();
	attr->id = get_id();
	attr->name = name;
	attr->is_input = input;
	attr->owner_id = owner_id;
	attr->values = std::vector<float>();
	attr->val_type = val_type;

	attributes[attr->id] = attr;
	return attr;
}

SurfaceProvider::SurfaceProvider() : mtx()
{	

	strcpy(path_buf, "");

	clear();

	dirty = false;
}

int SurfaceProvider::get_id()
{
	node_id++;
	return node_id;
}

void SurfaceProvider::clear()
{
	for (auto node : nodes)
	{
		if (node.second != NULL)
		{
			delete node.second;
		}
	}

	nodes.clear();

	for (auto attr : attributes)
	{
		if (attr.second != NULL)
		{
			delete attr.second;
		}
	}

	attributes.clear();

	links.clear();

	dirty = true;

	selected_node_type = "";
	unsaved_changes = false;
	// IDs lower or equal to 16 are reserved
	node_id = 16;

	// Push basic nodes
	nodes[OUTPUT_ID] = new OutputSPN();
	nodes[OUTPUT_ID]->id = OUTPUT_ID;
	nodes[OUTPUT_ID]->create(this);

	nodes[INPUT_ID] = new InputSPN();
	nodes[INPUT_ID]->id = INPUT_ID;
	nodes[INPUT_ID]->create(this);

	imnodes::SetNodePos(OUTPUT_ID, ImVec2(512, 128));
	imnodes::SetNodePos(INPUT_ID, ImVec2(32, 128));

	auto_preview = false;

	save_path = "";

}

void SurfaceProvider::do_preview(PreviewSPN* node, float radius)
{

	float y_step = glm::pi<float>() / (float)node->pixels_height;
	float x_step = glm::two_pi<float>() / (float)node->pixels_width;

	std::vector<float> sphere_val;
	std::vector<float> radius_val;
	std::vector<glm::vec3> positions;


	// Generate a whole sphere
	size_t y = 0;
	size_t x = 0;
	size_t width = node->pixels_width;
	size_t height = node->pixels_height;

	sphere_val.resize(width * height * 3);
	radius_val.resize(width * height);
	positions.resize(width * height);


	for (float inclination = 0.0f; inclination < glm::pi<float>(); inclination += y_step)
	{
		for (float azimuth = 0.0f; azimuth < glm::two_pi<float>(); azimuth += x_step)
		{
			glm::vec3 euclidean = MathUtil::spherical_to_euclidean_r1(azimuth, inclination);

			sphere_val[(y * width + x) * 3 + 0] = euclidean.x;
			sphere_val[(y * width + x) * 3 + 1] = euclidean.y;
			sphere_val[(y * width + x) * 3 + 2] = euclidean.z;
			radius_val[y * width + x] = radius;
			positions[y * width + x] = euclidean;

			x++;
		}
		x = 0;
		y++;
	}

	InputSPN* input = (InputSPN*)nodes[INPUT_ID];
	input->out_attribute[InputSPN::SPHERE_POS]->values = sphere_val;
	input->out_attribute[InputSPN::RADIUS]->values = radius_val;

	bool ret = node->propagate(this, node->pixels_width * node->pixels_height);

	if (ret == true)
	{
		complete = true;
		node->notify_preview_done(positions);
	}
	else
	{
		complete = false;
	}

}

void SurfaceProvider::get_heights(PlanetTilePath & path, size_t verts, std::vector<float>& out,
	glm::mat4 sphere_model, glm::mat4 cube_model, glm::mat4 path_model, float planetRadius)
{
	this->planetRadius = planetRadius;

	glm::dvec2 min = path.get_min();

	// Generate all basic arrays
	std::vector<float> sphere_val;
	std::vector<float> radius;

	size_t verts_plus_2 = verts + 2;

	sphere_val.resize((verts_plus_2 * verts_plus_2) * 3);

	radius.push_back(planetRadius);

	for (int iy = -1; iy < (int)verts + 1; iy++)
	{
		for (int ix = -1; ix < (int)verts + 1; ix++)
		{
			float x = (float)ix / (float)(verts - 1);
			float y = (float)iy / (float)(verts - 1);

			glm::vec3 sphere = get_sphere_pos(sphere_model, glm::vec2(x, y), path_model);

			sphere_val[((iy + 1) * verts_plus_2 + (ix + 1)) * 3 + 0] = sphere.x;
			sphere_val[((iy + 1) * verts_plus_2 + (ix + 1)) * 3 + 1] = sphere.y;
			sphere_val[((iy + 1) * verts_plus_2 + (ix + 1)) * 3 + 2] = sphere.z;
		}
	}

	OutputSPN* output = (OutputSPN*)nodes[OUTPUT_ID];

	// Feed basic data to Input
	InputSPN* input = (InputSPN*)nodes[INPUT_ID];
	input->out_attribute[InputSPN::SPHERE_POS]->values = sphere_val;
	input->out_attribute[InputSPN::RADIUS]->values = radius;

	// Call propagate on output, it will recursively obtain everything
	bool ret = output->propagate(this, verts_plus_2 * verts_plus_2);

	if (ret == true)
	{
		// Copy from output
		out = output->in_attribute[OutputSPN::HEIGHT]->values;
		complete = true;
	}
	else
	{
		complete = false;
	}

	// Do nothing, it could not be built
	
}