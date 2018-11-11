#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <iostream>
#include <rang.h>

class Shader;

namespace spd = spdlog;

typedef std::shared_ptr<spd::logger> Logger;

typedef unsigned int uint;

/*
	Please make sure this thing is always valid
	otherwise most GL stuff will ** probably **
	go haywire.
*/

extern Shader* g_shader;
extern Shader* d_shader;