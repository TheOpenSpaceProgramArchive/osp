#pragma once
#include <string>
#include "../../util/file_util.h"
#include <glad/glad.h>
#include <stb/stb_image.h>
#include "../../util/defines.h"

class Texture
{
public:

	GLuint texture;


	Texture(std::string path);
	~Texture();
};

