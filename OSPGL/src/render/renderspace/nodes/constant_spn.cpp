#include "constant_spn.h"
#include "../surface_provider.h"

bool ConstantSPN::do_imgui(int id)
{
	float old_val = value;
	ImGui::PushItemWidth(50.0f);
	ImGui::InputFloat("", &value);
	ImGui::PopItemWidth();
	if (old_val != value)
	{
		return true;
	}

	return false;
}

std::string ConstantSPN::get_name()
{
	return "Constant";
}

void ConstantSPN::process(size_t length)
{
	out_attribute[NUMBER]->values.clear();
	out_attribute[NUMBER]->values.resize(length);
	for (size_t i = 0; i < length; i++)
	{
		out_attribute[NUMBER]->values[i] = value;
	}
}

void ConstantSPN::create(SurfaceProvider* surf)
{
	out_attribute[NUMBER] = surf->create_attribute("Value", id, false);
}


