#pragma once
#include "../surface_provider_node.h"

// idname = Math
class MathSPN : public SurfaceProviderNode
{
public:

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

	float val_a;
	float val_b;

	static constexpr int INPUT_A = 0;
	static constexpr int INPUT_B = 1;
	static constexpr int OUTPUT = 2;

	virtual bool do_imgui(int id);
	virtual std::string get_name() override;
	virtual void process(size_t length) override;
	virtual void create(SurfaceProvider * surf) override;
};