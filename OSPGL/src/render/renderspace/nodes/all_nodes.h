#pragma once
#include "output_spn.h"
#include "input_spn.h"
#include "constant_spn.h"
#include "math_spn.h"
#include "noise_spn.h"
#include "pack_spn.h"
#include "unpack_spn.h"
#include "preview_spn.h"
#include <vector>

class SurfaceProvider;

std::vector<std::string> get_all_idnames();

// If index override is >= 0 then the node won't have create called, and will be assigned said id
SurfaceProviderNode* create_new_node(const std::string& idname, SurfaceProvider* provider, int index_override = -1);