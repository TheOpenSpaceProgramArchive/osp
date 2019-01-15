#pragma once

#include "../../../render/renderlow/mesh.h"
#include "../../../render/renderlow/framebuffer.h"
#include "../../../render/renderlow/texture.h"
#include "../../../render/renderlow/drawables/dsprite.h"
#include <tiny_obj_loader.h>

class Navball
{
public:

	Mesh navball_mesh;
	Framebuffer fbuffer;
	Shader navball_shader;
	Texture texture;
	DSprite frame_sprite;
	DSprite final_sprite;

	// Sprites for indicators
	Texture prograde_tex;
	Texture retrograde_tex;
	Texture normal_tex;
	Texture antinormal_tex;
	Texture radialin_tex;
	Texture radialout_tex;

	float view_distance = 4.0f;
	// Used to fit the navball inside the frame texture
	float scale = 0.85f;

	// Actual scale of the UI element
	float final_scale = 0.4f;

	// Scale of the markers, scaled when the markers get near the edges
	float icon_scale = 0.3f;

	// The coordinate it's given is what the navball
	// will show at its center (the direction of the vessel)
	void draw_to_texture(glm::quat rot, glm::quat prog);

	// Draws the navball to the screen centered bottom
	// directly
	void draw_to_screen(glm::ivec2 screen_size);

	Navball();
	~Navball();
};

