#pragma once
#include <glm/glm.hpp>
#include "../../orbital/planet.h"
#include <vector>
#include <stb/stb_perlin.h>

// Base class for an arbitrary surface provider 
// This one provides a totally flat planet, but gives an example
// on how to implement a surface provider, and provides some helper
// functions
class SurfaceProvider
{
public:


	
	// You should return an array containing vertsxverts heights, ordered using the method:
	// index = y * verts + x
	virtual void get_heights(PlanetTilePath& path, size_t verts, std::vector<float>& out, const Planet& planet)
	{
		glm::dvec2 min = path.get_min();

		for (size_t iy = 0; iy < verts; iy++)
		{
			for (size_t ix = 0; ix < verts; ix++)
			{
				float x = (float)ix / (float)(verts - 1);
				float y = (float)iy / (float)(verts - 1);

				glm::vec2 side = get_side_position(min, glm::vec2(x, y), path.getSize());

				float height = 0.0f;

				//height = sin(side.x * 400.0f) * 0.005f + sin(side.y * 350.0f) * 0.005f;
				//height = fmod(side.y * 10.0f, 1.0f) * 0.04f;

				/*if ((side.y >= 0.45f && side.y <= 0.55f) || (side.x >= 0.45f && side.x <= 0.55f))
				{
					height = 0.1f;
				}
				else
				{
					height = 0.0f;
				}*/

				//height = 0.0f;

				height = stb_perlin_ridge_noise3(side.x * 50.0f, side.y * 50.0f, 0.0f, 2.0f, 1.0f, 0.5f, 8) * 0.01f;

				//height = 0.0f;

				out[iy * verts + ix] = height;
			}
		}
	}
	
	// Gets absolute side position (0->1) from tile minimum and
	// normalized sub-tile coordinate (0->1)
	glm::vec2 get_side_position(glm::dvec2 min, glm::vec2 coord, double size)
	{
		return glm::vec2(min.x, min.y) + coord * (float)size;
	}

};