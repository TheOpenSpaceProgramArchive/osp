#pragma once
#include "../surface_provider_node.h"

// idname = Output
class OutputSPN : public SurfaceProviderNode
{
public:

	static constexpr int HEIGHT		= 0;

	virtual std::string get_name() override;
	virtual void process(size_t length) override;
	virtual void create(SurfaceProvider * surf) override;
};