#include "all_nodes.h"
#include "../surface_provider.h"

std::vector<std::string> get_all_idnames()
{
	std::vector<std::string> out;

	// output_spn.h (Disabled, it's default node)
	// out.push_back("Output");

	// input_spn.h (Disabled, it's a default node)
	// out.push_back("Input");

	// constant_spn.h
	out.push_back("Constant");

	// math_spn.h
	out.push_back("Math");

	//noise_spn.h
	out.push_back("Noise");

	return out;
}

SurfaceProviderNode* create_new_node(const std::string& idname, SurfaceProvider* provider)
{
	SurfaceProviderNode* out;

	if (idname == "Output")
	{
		out = new OutputSPN();
	}
	else if (idname == "Input")
	{
		out = new InputSPN();
	}
	else if (idname == "Constant")
	{
		out = new ConstantSPN();
	}
	else if (idname == "Math")
	{
		out = new MathSPN();
	}
	else if (idname == "Noise")
	{
		out = new NoiseSPN();
	}
	else
	{
		out = NULL;
	}

	if (out != NULL)
	{
		out->id = provider->get_id();
		out->create(provider);
	}

	return out;
}