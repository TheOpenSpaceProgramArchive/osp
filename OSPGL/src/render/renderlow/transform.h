#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

/*
	A transform stored the position, rotation
	and scale.

	Uses a quaternion to store rotations but gives
	a lot of utility functions.
*/
class Transform
{
public:

	Transform* parent = NULL;

	glm::vec3 pos = glm::vec3(0, 0, 0);
	glm::vec3 scl = glm::vec3(1, 1, 1);
	glm::quat rot;

	// Not updated unless build_matrix is called
	glm::mat4 mat;

	glm::mat4 build_matrix();

	Transform();
	~Transform();
};

