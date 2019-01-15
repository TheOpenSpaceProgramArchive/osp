#pragma once
#include <string>
#include "../../util/file_util.h"
#include <glad/glad.h>
#include <stb/stb_image.h>
#include "../../util/defines.h"
#include <glm/glm.hpp>

class Texture
{
private:
	glm::ivec2 size;

public:

	GLuint texture;

	glm::ivec2 get_size();

	Texture(GLuint tex);
	Texture(std::string path);
	~Texture();
};

