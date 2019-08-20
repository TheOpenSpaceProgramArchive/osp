#pragma once
#include "output_spn.h"
#include "input_spn.h"
#include "constant_spn.h"
#include "math_spn.h"
#include "noise_spn.h"
#include <vector>

class SurfaceProvider;

std::vector<std::string> get_all_idnames();

SurfaceProviderNode* create_new_node(const std::string& idname, SurfaceProvider* provider);