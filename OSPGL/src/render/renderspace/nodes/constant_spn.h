#pragma once
#include "../surface_provider_node.h"

// idname = Constant
class ConstantSPN : public SurfaceProviderNode
{
public:

	float value;

	static constexpr int NUMBER = 0;

	virtual bool do_imgui(int id);

	virtual std::string get_name() override;
	virtual void process(size_t length) override;
	virtual void create(SurfaceProvider * surf) override;
};