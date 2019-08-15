#pragma once
#include <glm/glm.hpp>
#include "../../orbital/planet.h"
#include "../../util/math_util.h"
#include <vector>
#include <stb/stb_perlin.h>
#include <imgui/imgui.h>
#include "planet_tile_path.h"

// Base class for an arbitrary surface provider 
// This one provides a totally flat planet, but gives an example
// on how to implement a surface provider, and provides some helper
// functions
class SurfaceProvider
{
public:



	// You should return an array containing (verts + 1)x(verts + 1) floats, indicating height in meters
	// from "sea level"
	// Indexing is done via y * (verts + 1) + x
	//
	virtual void get_heights(PlanetTilePath& path, size_t verts, std::vector<float>& out, const Planet& planet,
		glm::mat4 sphere_model, glm::mat4 cube_model, glm::mat4 path_model)
	{
		glm::dvec2 min = path.get_min();

		for (int iy = -1; iy < (int)verts + 1; iy++)
		{
			for (int ix = -1; ix < (int)verts + 1; ix++)
			{
				float x = (float)ix / (float)(verts - 1);
				float y = (float)iy / (float)(verts - 1);

				glm::vec2 side = get_side_position(min, glm::vec2(x, y), path.getSize());
				glm::vec3 sphere = get_sphere_pos(sphere_model, glm::vec2(x, y), path_model);

				float height = 0.0f;

				height = stb_perlin_ridge_noise3(sphere.x * 250.0f, sphere.y * 250.0f, sphere.z * 250.0f, 2.0f, 0.7f, 0.9f, 8) * 0.0005f +
					stb_perlin_fbm_noise3(sphere.x * 58.0f, sphere.y * 58.0f, sphere.z * 58.0f, 2.0f, 0.5f, 8) * 0.00002f +
					stb_perlin_fbm_noise3(sphere.x * 3.0f, sphere.y * 3.0f, sphere.z * 3.0f, 2.0f, 0.5f, 8) * 0.035f +
					stb_perlin_turbulence_noise3(sphere.x * 8.0f, sphere.y * 16.0f, sphere.z * 16.0f, 2.0f, 0.5f, 8) * 0.005f -
					stb_perlin_ridge_noise3(sphere.x * 8.0f, sphere.y * 8.0f, sphere.z * 8.0f, 2.0f, 0.5f, 0.6f, 8) * 0.035f +
					stb_perlin_fbm_noise3(sphere.x * 600.0f, sphere.y * 600.0f, sphere.z * 600.0f, 3.0f, 0.6f, 8) * 0.000005f;

				// Always add, not overwrite, so you are compatible with the layer system
				out[(iy + 1) * (verts + 2) + (ix + 1)] += height;
			}
		}
	}

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
	virtual void draw_imgui()
	{
		ImGui::Text("(Not editable)");
	}

	virtual std::string get_imgui_title()
	{
		return "Not-Overriden Surface";
	}
};