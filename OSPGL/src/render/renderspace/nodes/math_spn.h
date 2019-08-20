#pragma once
#include "../surface_provider_node.h"
#include <glm/glm.hpp>

// idname = Math
class MathSPN : public SurfaceProviderNode
{
public:

	bool show_warning;

	SurfaceProvider* surf;

	enum MathOperation
	{
		ADD,			// R = A + B
		SUBSTRACT,		// R = A - B
		MULTIPLY,		// R = A * B
		DIVIDE,			// R = A / B
		MODULO,			// R = A % B
		POWER,			// R = A ^ B	

		END_MARKER
	};

	MathOperation operation;

	glm::vec3 val_a;
	glm::vec3 val_b;

	ValueType valtype_a = ValueType::V1;
	ValueType valtype_b = ValueType::V1;

	static constexpr int INPUT_A = 0;
	static constexpr int INPUT_B = 1;
	static constexpr int OUTPUT = 2;

	virtual bool do_imgui(int id);
	virtual std::string get_name() override;
	virtual void process(size_t length) override;
	virtual void create(SurfaceProvider * surf) override;
};