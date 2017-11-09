#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <iostream>
#include <rang.h>

namespace spd = spdlog;

typedef std::shared_ptr<spd::logger> logger;

typedef unsigned int uint;