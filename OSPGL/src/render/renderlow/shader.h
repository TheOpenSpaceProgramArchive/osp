#pragma once

#include <string>
#include "../../util/file_util.h"
#include <glad/glad.h>

#include "../../util/defines.h"

#include <string>

class shader
{
public:

	int program;

	logger log;

	void load(std::string vertex, std::string fragment);

	shader(std::string vertex, std::string fragment);
	shader();
	~shader();
};

