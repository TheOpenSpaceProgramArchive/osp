#pragma once

#include "../../../render/renderlow/mesh.h"
#include "../../../render/renderlow/framebuffer.h"
#include "../../../render/renderlow/texture.h"
#include <tiny_obj_loader.h>

class Navball
{
public:

	Mesh navball_mesh;
	Framebuffer fbuffer;
	Shader navball_shader;
	Texture texture;

	float view_distance = 2.5f;

	// The coordinate it's given is what the navball
	// will show at its center (the direction of the vessel)
	void draw_to_texture(glm::vec3 forward);

	Navball();
	~Navball();
};

