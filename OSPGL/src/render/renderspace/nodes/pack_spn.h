#pragma once
#include "../surface_provider_node.h"

// idname = Pack
class PackSPN : public SurfaceProviderNode
{
public:

	static constexpr int INPUT_A	= 0;
	static constexpr int INPUT_B	= 1;
	static constexpr int INPUT_C	= 2;
	static constexpr int OUTPUT		= 3;

	virtual bool do_imgui(int id);
	virtual std::string get_name() override;
	virtual void process(size_t length) override;
	virtual void create(SurfaceProvider * surf) override;
};