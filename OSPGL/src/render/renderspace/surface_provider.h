#pragma once
#include <glm/glm.hpp>
#include "../../util/math_util.h"
#include <vector>
#include <stb/stb_perlin.h>
#include <imgui/imgui.h>
#include "planet_tile_path.h"
#include <unordered_map>
#include "surface_provider_node.h"
#include "nodes/all_nodes.h"
#include <json.hpp>

using namespace nlohmann;

// A surface provider allows stacking many surface layers and combining their outputs and inputs arbitrarly
class SurfaceProvider
{
private:

	int node_id;

	std::string selected_node_type;

	bool complete = false;

	int selected_id = -1;

	bool auto_preview;

	float planetRadius;

	std::string selected_file;
	std::string save_path;

	bool unsaved_changes;
	char path_buf[512];

public:

	// TODO: Maybe create a of surface provider copy for each thread? 
	// Could be a decent optimization
	std::recursive_mutex mtx;

	bool dirty = false;


	static constexpr int INPUT_ID = 2;
	static constexpr int OUTPUT_ID = 1;

	std::unordered_map<int, SurfaceProviderNode*> nodes;
	std::unordered_map<int, SurfaceProviderAttribute*> attributes;

	std::vector<std::pair<int, int>> links;

	void do_preview(PreviewSPN* node, float radius);

	// You should return an array containing (verts + 1)x(verts + 1) floats, indicating height in meters
	// from "sea level"
	// Indexing is done via y * (verts + 1) + x
	void get_heights(PlanetTilePath& path, size_t verts, std::vector<float>& out,
		glm::mat4 sphere_model, glm::mat4 cube_model, glm::mat4 path_model, float planet_radius);

	// Gets absolute side position (0->1) from tile minimum and
	// normalized sub-tile coordinate (0->1)
	static glm::vec2 get_side_position(glm::dvec2 min, glm::vec2 coord, double size)
	{
		return glm::vec2(min.x, min.y) + coord * (float)size;
	}

	// Gets position in the normalized sphere
	static glm::vec3 get_sphere_pos(glm::mat4 sphere_model, glm::vec2 coord, glm::mat4 path_model)
	{
		glm::vec3 world_pos_cubic = path_model * glm::vec4(glm::vec3(coord, 0.0f), 1.0f);
		glm::vec3 world_pos_spheric = MathUtil::cube_to_sphere(world_pos_cubic);

		return world_pos_spheric;
	}

	// Draw ImGui widgets for editing
	void draw_imgui();

	json serialize(bool save_imnodes = true);
	void deserialize(json j, bool load_imnodes = true);

	void save();

	SurfaceProviderAttribute* create_attribute(std::string name, int owner_id, bool input, ValueType val_type);

	SurfaceProvider();
	
	int get_id();

	void clear();

};