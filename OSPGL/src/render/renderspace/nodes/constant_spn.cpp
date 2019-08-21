#include "constant_spn.h"
#include "../surface_provider.h"

bool ConstantSPN::do_imgui(int id)
{
	bool set_dirty = false;

	glm::vec3 old_val = val;
	ImGui::PushItemWidth(180.0f);
	val_type = pick_val_type(val_type, &set_dirty);
	if (val_type == V1)
	{
		ImGui::InputFloat("", &val.x);
	}
	else if (val_type == V2)
	{
		ImGui::InputFloat2("", &val.x);
	}
	else
	{
		ImGui::InputFloat3("", &val.x);
	}
	
	out_attribute[NUMBER]->val_type = val_type;

	ImGui::PopItemWidth();
	if (old_val != val)
	{
		set_dirty = true;
	}

	return set_dirty;
}

std::string ConstantSPN::get_name()
{
	return "Constant";
}

void ConstantSPN::process(size_t length)
{
	out_attribute[NUMBER]->values.clear();
	size_t val_size;
	if (val_type == V1)
	{
		val_size = 1;
	}
	else if (val_type == V2) 
	{
		val_size = 2;
	}
	else
	{
		val_size = 3;
	}

	out_attribute[NUMBER]->values.resize(length * val_size);
	for (size_t i = 0; i < length; i++)
	{
		for (size_t j = 0; j < val_size; j++)
		{
			out_attribute[NUMBER]->values[i * val_size + j] = val[j];
		}
		
	}
}

void ConstantSPN::create(SurfaceProvider* surf)
{
	out_attribute[NUMBER] = surf->create_attribute("Value", id, false, ANY);
}

using namespace nlohmann;

json ConstantSPN::serialize()
{
	json out;

	out["x"] = val.x;
	out["y"] = val.y;
	out["z"] = val.z;

	out["val_type"] = val_type;

	return out;
}

void ConstantSPN::deserialize(json j)
{
	val.x = j["x"];
	val.y = j["y"];
	val.z = j["z"];

	val_type = j["val_type"];
}


