#include "surface_provider.h"
#include <imnodes/imnodes.h>
#include "nodes/all_nodes.h"


void SurfaceProvider::draw_imgui()
{
	if (ImGui::BeginPopupModal("New Node", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::string node_type_str = "Select Node Type";
		if (selected_node_type != "")
		{
			node_type_str = selected_node_type;
		}

		if (ImGui::BeginCombo("", node_type_str.c_str()))
		{
			std::vector<std::string> types = get_all_idnames();
			for (const std::string& type : types)
			{
				if (ImGui::Selectable(type.c_str()))
				{
					selected_node_type = type;
				}
			}

			ImGui::EndCombo();
		}

	
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
			selected_node_type = "";
		}
		ImGui::SameLine();
		if (ImGui::Button("Add"))
		{
			ImGui::CloseCurrentPopup();
			if (selected_node_type != "")
			{
				SurfaceProviderNode* new_node = create_new_node(selected_node_type, this);
				nodes[new_node->id] = new_node;
				selected_node_type = "";
			}
		}

		ImGui::EndPopup();
	}

	if (ImGui::Button("Create New"))
	{
		ImGui::OpenPopup("New Node");
	}

	ImGui::SameLine();

	if (last_frame_selected_id > 16)
	{
		if (ImGui::Button("Delete Selected"))
		{
			std::vector<size_t> to_remove;

			for (auto it = links.begin(); it != links.end();)
			{
				if (it->first == last_frame_selected_id || it->second == last_frame_selected_id)
				{
					it = links.erase(it);
				}
				else
				{
					it++;
				}
			}

			SurfaceProviderNode* node = nodes[last_frame_selected_id];

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

			delete nodes[last_frame_selected_id];
			nodes.erase(last_frame_selected_id);
		}
	}
	else
	{
		ImGui::TextDisabled("No deletable node selected");
	}
	

	ImGui::SameLine();

	if (complete)
	{
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Complete!");
	}
	else
	{
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Nodes not satisfied, check inputs");
	}


	imnodes::BeginNodeEditor();

	for (auto node : nodes)
	{
		imnodes::BeginNode(node.first);
		imnodes::Name(node.second->get_name().c_str());

		for (auto attr : node.second->in_attribute)
		{
			imnodes::BeginInputAttribute(attr.second->id);
			ImGui::Text("%s", attr.second->name.c_str());
			if (node.second->do_imgui(attr.first))
			{
				dirty = true;
			}
			
			imnodes::EndAttribute();
		}

		for (auto attr : node.second->out_attribute)
		{
			imnodes::BeginOutputAttribute(attr.second->id);
			ImGui::Text("%s", attr.second->name.c_str());
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
		imnodes::Link(i, links[i].first, links[i].second);
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
	if (imnodes::IsLinkHovered(&link_index) && ImGui::IsMouseDown(1) && imnodes::IsLinkSelected(&link_sel_index) && link_index == link_sel_index)
	{
		auto link = links[link_index];
		attributes[link.first]->remove_link(link.second);
		attributes[link.second]->remove_link(link.first);

		links.erase(links.begin() + link_index);

		dirty = true;
	}


	last_frame_selected_id = selected_id;

	int node_id;
	if (imnodes::IsNodeSelected(&node_id))
	{
		selected_id = node_id;
	}
	else
	{
		selected_id = -1;
	}


}

SurfaceProviderAttribute* SurfaceProvider::create_attribute(std::string name, int owner_id, bool input)
{

	SurfaceProviderAttribute* attr = new SurfaceProviderAttribute();
	attr->id = get_id();
	attr->name = name;
	attr->is_input = input;
	attr->owner_id = owner_id;
	attr->values = std::vector<float>();

	attributes[attr->id] = attr;
	return attr;
}

SurfaceProvider::SurfaceProvider()
{
	selected_node_type = "";
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
}

int SurfaceProvider::get_id()
{
	node_id++;
	return node_id;
}

void SurfaceProvider::get_heights(PlanetTilePath & path, size_t verts, std::vector<float>& out, 
	glm::mat4 sphere_model, glm::mat4 cube_model, glm::mat4 path_model, float planetRadius)
{
	glm::dvec2 min = path.get_min();

	// Generate all basic arrays
	std::vector<float> sphere_x;
	std::vector<float> sphere_y;
	std::vector<float> sphere_z;
	std::vector<float> radius;

	size_t verts_plus_2 = verts + 2;

	sphere_x.resize(verts_plus_2 * verts_plus_2);
	sphere_y.resize(verts_plus_2 * verts_plus_2);
	sphere_z.resize(verts_plus_2 * verts_plus_2);

	radius.push_back(planetRadius);

	for (int iy = -1; iy < (int)verts + 1; iy++)
	{
		for (int ix = -1; ix < (int)verts + 1; ix++)
		{
			float x = (float)ix / (float)(verts - 1);
			float y = (float)iy / (float)(verts - 1);

			glm::vec3 sphere = get_sphere_pos(sphere_model, glm::vec2(x, y), path_model);

			sphere_x[(iy + 1) * verts_plus_2 + (ix + 1)] = sphere.x;
			sphere_y[(iy + 1) * verts_plus_2 + (ix + 1)] = sphere.y;
			sphere_z[(iy + 1) * verts_plus_2 + (ix + 1)] = sphere.z;
		}
	}

	OutputSPN* output = (OutputSPN*)nodes[OUTPUT_ID];

	// Feed basic data to Input
	InputSPN* input = (InputSPN*)nodes[INPUT_ID];
	input->out_attribute[InputSPN::SPHERE_X]->values = sphere_x;
	input->out_attribute[InputSPN::SPHERE_Y]->values = sphere_y;
	input->out_attribute[InputSPN::SPHERE_Z]->values = sphere_z;
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