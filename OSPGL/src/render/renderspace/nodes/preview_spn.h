#pragma once
#include "../surface_provider_node.h"
#include "../../renderlow/image.h"
#include "../../renderlow/texture.h"

// idname = Preview
class PreviewSPN : public SurfaceProviderNode
{
public:

	Image img;
	Texture tex;

	int pixels_width = 128;
	int pixels_height = 80;

	static constexpr int HEIGHT = 0;

	virtual std::string get_name() override;
	virtual void process(size_t length) override;
	virtual void create(SurfaceProvider * surf) override;

	virtual bool do_imgui(int id);

	// Each position shares an index with a height, that's how we map to the 
	// 2D-plane
	void notify_preview_done(const std::vector<glm::vec3>& positions);
};