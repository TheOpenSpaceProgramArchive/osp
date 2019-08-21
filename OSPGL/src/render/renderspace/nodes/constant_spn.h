#pragma once
#include "../surface_provider_node.h"
#include <glm/glm.hpp>

// idname = Constant
class ConstantSPN : public SurfaceProviderNode
{
public:

	glm::vec3 val;
	ValueType val_type;

	static constexpr int NUMBER = 0;

	virtual bool do_imgui(int id);

	virtual std::string get_name() override;
	virtual void process(size_t length) override;
	virtual void create(SurfaceProvider * surf) override;

	virtual nlohmann::json serialize() override;
	virtual void deserialize(nlohmann::json j) override;
};