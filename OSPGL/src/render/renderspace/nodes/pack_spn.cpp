#include "pack_spn.h"
#include "../surface_provider.h"

bool PackSPN::do_imgui(int id)
{
	if (id == INPUT_C)
	{
		if (in_attribute[INPUT_C]->links.size() == 0)
		{
			in_attribute[INPUT_C]->has_values = true;
			out_attribute[OUTPUT]->val_type = V2;
			ImGui::SameLine();
			ImGui::TextDisabled("(D)");
		}
		else
		{
			in_attribute[INPUT_C]->has_values = false;
			out_attribute[OUTPUT]->val_type = V3;
		}
	}

	return false;
}

std::string PackSPN::get_name()
{
	return "Pack";
}

void PackSPN::process(size_t length)
{
	std::vector<float>* x = &in_attribute[INPUT_A]->values;
	std::vector<float>* y = &in_attribute[INPUT_B]->values;
	std::vector<float>* z = &in_attribute[INPUT_C]->values;

	if (in_attribute[INPUT_C]->has_values)
	{
		out_attribute[OUTPUT]->values.resize(length * 2);
		for (size_t i = 0; i < length; i++)
		{
			out_attribute[OUTPUT]->values[i * 2 + 0] = (*x)[i];
			out_attribute[OUTPUT]->values[i * 2 + 1] = (*y)[i];
		}
	}
	else
	{
		out_attribute[OUTPUT]->values.resize(length * 3);
		for (size_t i = 0; i < length; i++)
		{
			out_attribute[OUTPUT]->values[i * 3 + 0] = (*x)[i];
			out_attribute[OUTPUT]->values[i * 3 + 1] = (*y)[i];
			out_attribute[OUTPUT]->values[i * 3 + 2] = (*z)[i];
		}
	}
}

void PackSPN::create(SurfaceProvider* surf)
{
	in_attribute[INPUT_A] = surf->create_attribute("X", id, true, V1);
	in_attribute[INPUT_B] = surf->create_attribute("Y", id, true, V1);
	in_attribute[INPUT_C] = surf->create_attribute("Z", id, true, V1);
	out_attribute[OUTPUT] = surf->create_attribute("O", id, false, ANY);
}
