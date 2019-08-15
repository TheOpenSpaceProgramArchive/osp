#pragma once
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
class MathUtil
{
public:

	// Takes and outputs normalized coordinates
	static glm::vec3 cube_to_sphere(glm::vec3 cubic);
	// Takes and outputs normalized coordinates
	static glm::vec3 sphere_to_cube(glm::vec3 spheric);

	static glm::mat4 rotate_from_to(glm::vec3 from, glm::vec3 to);
};