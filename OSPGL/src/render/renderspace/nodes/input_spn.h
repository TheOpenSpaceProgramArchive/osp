#pragma once
#include "../surface_provider_node.h"

// idname = Input
class InputSPN : public SurfaceProviderNode
{
public:

	static constexpr int SPHERE_POS = 0;
	static constexpr int RADIUS		= 1;


	virtual std::string get_name() override;
	virtual void process(size_t length) override;
	virtual void create(SurfaceProvider * surf) override;
};