#pragma once
#include <glm/glm.hpp>

class MathUtil
{
public:

	// Takes and outputs normalized coordinates
	static glm::vec3 cube_to_sphere(glm::vec3 cubic);
	// Takes and outputs normalized coordinates
	static glm::vec3 sphere_to_cube(glm::vec3 spheric);
};