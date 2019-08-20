#include "unpack_spn.h"
#include "../surface_provider.h"

bool UnpackSPN::do_imgui(int id)
{
	SurfaceProviderAttribute* linked = NULL;
	if (in_attribute[INPUT]->links.size() != 0)
	{
		linked = surf->attributes[in_attribute[INPUT]->links[0]];
	}

	if (linked != NULL)
	{
		in_attribute[INPUT]->val_type = linked->val_type;
	}
	else
	{
		in_attribute[INPUT]->val_type = ANY;
	}

	if (id == OUTPUT_Y)
	{
		if (linked != NULL && linked->val_type == V1)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("(0)");
		}
	}
	else if (id == OUTPUT_Z) 
	{
		if (linked != NULL && linked->val_type == V2)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("(0)");
		}
	}

	return false;
}

std::string UnpackSPN::get_name()
{
	return "Unpack";
}

void UnpackSPN::process(size_t length)
{
	SurfaceProviderAttribute* linked = NULL;
	if (in_attribute[INPUT]->links.size() != 0)
	{
		linked = surf->attributes[in_attribute[INPUT]->links[0]];
	}

	out_attribute[OUTPUT_X]->values.resize(length);
	out_attribute[OUTPUT_Y]->values.resize(length);
	out_attribute[OUTPUT_Z]->values.resize(length);

	if (linked->val_type == V1)
	{
		for (size_t i = 0; i < length; i++)
		{
			out_attribute[OUTPUT_X]->values[i] = in_attribute[INPUT]->values[i];
			out_attribute[OUTPUT_Y]->values[i] = 0.0f;
			out_attribute[OUTPUT_Z]->values[i] = 0.0f;
		}
	}
	else if (linked->val_type == V2)
	{
		for (size_t i = 0; i < length; i++)
		{
			out_attribute[OUTPUT_X]->values[i] = in_attribute[INPUT]->values[i * 2 + 0];
			out_attribute[OUTPUT_Y]->values[i] = in_attribute[INPUT]->values[i * 2 + 1];
			out_attribute[OUTPUT_Z]->values[i] = 0.0f;
		}
	}
	else
	{
		for (size_t i = 0; i < length; i++)
		{
			out_attribute[OUTPUT_X]->values[i] = in_attribute[INPUT]->values[i * 3 + 0];
			out_attribute[OUTPUT_Y]->values[i] = in_attribute[INPUT]->values[i * 3 + 1];
			out_attribute[OUTPUT_Z]->values[i] = in_attribute[INPUT]->values[i * 3 + 2];
		}
	}
	
}

void UnpackSPN::create(SurfaceProvider* surf)
{
	this->surf = surf;

	in_attribute[INPUT] = surf->create_attribute("I", id, true, ANY);
	out_attribute[OUTPUT_X] = surf->create_attribute("X", id, false, V1);
	out_attribute[OUTPUT_Y] = surf->create_attribute("Y", id, false, V1);
	out_attribute[OUTPUT_Z] = surf->create_attribute("Z", id, false, V1);
}
