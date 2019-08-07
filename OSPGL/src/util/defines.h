#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <iostream>
#include <rang.h>

#include "../game/ui/ui_manager.h"

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
extern UIManager ui_manager;

extern double scroll_delta;

// Scale between OpenGL and real units (Value is divided by this)
#define ORBIT_VIEW_SCALE (10e7)

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}