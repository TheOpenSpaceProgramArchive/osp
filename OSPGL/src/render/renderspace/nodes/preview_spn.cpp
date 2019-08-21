#include "preview_spn.h"
#include "../surface_provider.h"

std::string PreviewSPN::get_name()
{
	return "Preview";
}

void PreviewSPN::process(size_t length)
{

}

void PreviewSPN::create(SurfaceProvider* surf)
{
	in_attribute[HEIGHT] = surf->create_attribute("Height", id, true, V1);
}

bool PreviewSPN::do_imgui(int id)
{
	if (id == HEIGHT)
	{
		if (tex.is_uploaded())
		{
			ImGui::Image((void*)(intptr_t)tex.texture, ImVec2(img.width * 3, img.height * 3));
		}
		else
		{
			ImGui::Image(0, ImVec2(img.width, img.height));
		}
	}

	return false;
}

void PreviewSPN::notify_preview_done(const std::vector<glm::vec3>& positions)
{
	img.pixels.clear();
	img.pixels.resize(pixels_width * pixels_height);
	img.width = pixels_width;
	img.height = pixels_height;

	std::vector<float>* heights = &in_attribute[HEIGHT]->values;

	// Generate a bitmap using the wanted mapping
	for (int y = 0; y < pixels_height; y++)
	{
		for (int x = 0; x < pixels_width; x++)
		{
			float r = 0.0f; float g = 0.0f; float b = 0.0f;

			float height = (*heights)[y * pixels_width + x];

			r = height; g = height; b = 1.0f;

			r = glm::max(r, 0.0f); g = glm::max(g, 0.0f); b = glm::max(b, 0.0f);
			r = glm::min(r, 1.0f); g = glm::min(g, 1.0f); b = glm::min(b, 1.0f);
			img.set_pixel(x, y, { (uint8_t)(r * 255), (uint8_t)(g * 255), (uint8_t)(b * 255), 255 });
		}
	}

	if (tex.is_uploaded())
	{
		tex.unload();
	}

	tex = Texture(img);
}


