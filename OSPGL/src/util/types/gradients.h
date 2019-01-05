#pragma once

#include <glm/glm.hpp>
#include <map>

class Gradient
{
public:
	std::map<float, glm::vec4> keys;

	glm::vec4 evaluate(float t);
};