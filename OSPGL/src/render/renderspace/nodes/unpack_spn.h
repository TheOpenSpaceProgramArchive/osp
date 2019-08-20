#pragma once
#include "../surface_provider_node.h"

// idname = Unpack
class UnpackSPN : public SurfaceProviderNode
{
public:

	SurfaceProvider* surf;

	static constexpr int INPUT = 0;
	static constexpr int OUTPUT_X = 1;
	static constexpr int OUTPUT_Y = 2;
	static constexpr int OUTPUT_Z = 3;

	virtual bool do_imgui(int id);
	virtual std::string get_name() override;
	virtual void process(size_t length) override;
	virtual void create(SurfaceProvider * surf) override;
};