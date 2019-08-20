#include "math_spn.h"
#include "../surface_provider.h"

std::string op_to_str(MathSPN::MathOperation operation)
{
	std::string str = "ERROR";

	if (operation == MathSPN::ADD)
	{
		str = "A + B";
	}
	else if (operation == MathSPN::SUBSTRACT)
	{
		str = "A - B";
	}
	else if (operation == MathSPN::MULTIPLY)
	{
		str = "A * B";
	}
	else if (operation == MathSPN::DIVIDE)
	{
		str = "A / B";
	}
	else if (operation == MathSPN::MODULO)
	{
		str = "A % B";
	}
	else if (operation == MathSPN::POWER)
	{
		str = "A ^ B";
	}

	return str;
}



bool MathSPN::do_imgui(int id)
{
	glm::vec3 old_a = val_a;
	glm::vec3 old_b = val_b;

	ImGui::PushItemWidth(180.0f);

	bool set_dirty = false;

	// Obtain types for inputs
	if (id == INPUT_A) 
	{
		if (in_attribute[INPUT_A]->links.size() == 0)
		{
			ImGui::SameLine();
			valtype_a = pick_val_type(valtype_a, &set_dirty);

			in_attribute[INPUT_A]->has_values = true;
			if (valtype_a == V1)
			{
				set_dirty |= ImGui::InputFloat("", &val_a.x);
			}
			else if (valtype_a == V2)
			{
				set_dirty |= ImGui::InputFloat2("", &val_a.x);
			}
			else
			{
				set_dirty |= ImGui::InputFloat3("", &val_a.x);
			}
		}
		else 
		{
			in_attribute[INPUT_A]->has_values = false;
			valtype_a = surf->attributes[in_attribute[INPUT_A]->links[0]]->val_type;
		}
	}
	else if (id == INPUT_B)
	{
		if (in_attribute[INPUT_B]->links.size() == 0)
		{
			ImGui::SameLine();
			valtype_b = pick_val_type(valtype_b, &set_dirty);

			in_attribute[INPUT_B]->has_values = true;
			if (valtype_b == V1)
			{
				set_dirty |= ImGui::InputFloat("", &val_b.x);
			}
			else if (valtype_b == V2)
			{
				set_dirty |= ImGui::InputFloat2("", &val_b.x);
			}
			else
			{
				set_dirty |= ImGui::InputFloat3("", &val_b.x);
			}
		}
		else
		{
			in_attribute[INPUT_B]->has_values = false;
			valtype_b = surf->attributes[in_attribute[INPUT_B]->links[0]]->val_type;
		}
	}

	in_attribute[INPUT_A]->val_type = valtype_a;
	in_attribute[INPUT_B]->val_type = valtype_b;

	ImGui::PopItemWidth();

	// Find type of output
	int max_val = 0;
	for (int i = 0; i < ValueType::ANY; i++)
	{
		if (in_attribute[INPUT_A]->val_type == i || in_attribute[INPUT_B]->val_type == i)
		{
			if (i > max_val)
			{
				max_val = i;
			}
		}
	}

	out_attribute[OUTPUT]->val_type = ValueType(max_val);

	if (id == OUTPUT)
	{
		MathOperation old_op = operation;
		std::string str = op_to_str(operation);
		ImGui::SameLine();
		ImGui::PushItemWidth(60.0f);
		if (ImGui::BeginCombo("", str.c_str()))
		{

			for (int i = 0; i < MathOperation::END_MARKER; i++)
			{
				if (ImGui::Selectable(op_to_str((MathOperation)i).c_str()))
				{
					operation = (MathOperation)i;
				}
			}

			ImGui::EndCombo();
		}

		if (show_warning)
		{
			ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Warning, incompatible types!");
		}

		ImGui::PopItemWidth();

		if (operation != old_op)
		{
			set_dirty = true;
		}
	}

	if (val_a != old_a || val_b != old_b)
	{
		set_dirty = true;
	}

	return set_dirty;
}

std::string MathSPN::get_name()
{
	return "Math";
}

void operate(float* o, float a, float b, MathSPN::MathOperation operation)
{
	if (operation == MathSPN::ADD)
	{
		*o = a + b;
	}
	else if (operation == MathSPN::SUBSTRACT)
	{
		*o = a - b;
	}
	else if (operation == MathSPN::MULTIPLY)
	{
		*o = a * b;
	}
	else if (operation == MathSPN::DIVIDE)
	{
		*o = a / b;
	}
	else if (operation == MathSPN::MODULO)
	{
		*o = fmod(a, b);
	}
	else if (operation == MathSPN::POWER)
	{
		*o = pow(a, b);
	}
}

void MathSPN::process(size_t length)
{
	show_warning = false;

	SurfaceProviderAttribute* a = in_attribute[INPUT_A];
	SurfaceProviderAttribute* b = in_attribute[INPUT_B];
	SurfaceProviderAttribute* o = out_attribute[OUTPUT];


	int max_vals = 1;

	if (a->val_type == V1)
	{
		a->values.resize(length);
	}
	else if (a->val_type == V2)
	{
		a->values.resize(length * 2);
		if (2 > max_vals)
		{
			max_vals = 2;
		}
	}
	else
	{
		a->values.resize(length * 3);
		if (3 > max_vals)
		{
			max_vals = 3;
		}
	}

	if (b->val_type == V1)
	{
		b->values.resize(length);
	}
	else if (b->val_type == V2)
	{
		b->values.resize(length * 2);
		if (2 > max_vals)
		{
			max_vals = 2;
		}
	}
	else
	{
		b->values.resize(length * 3);
		if (3 > max_vals)
		{
			max_vals = 3;
		}
	}

	o->values.resize(length * max_vals);

	std::vector<float>* av = &a->values;
	std::vector<float>* bv = &b->values;
	std::vector<float>* ov = &o->values;

	// If V1 is a type, and the other is V2 or V3,
	// it's used as a scalar
	// If V2 is a type, and the other is V3
	// the operation is not performed, and a warning
	// is shown
	if (a->val_type == b->val_type)
	{
		for (size_t i = 0; i < length * max_vals; i++)
		{
			float* o = &(*ov)[i];
			float a = (*av)[i];
			float b = (*bv)[i];
			operate(o, a, b, operation);
		}
	}
	else if (a->val_type == V1 && b->val_type != V1)
	{
		// A as a scalar
		for (size_t i = 0; i < length; i++)
		{
			for (size_t j = 0; j < max_vals; j++)
			{
				float* o = &(*ov)[i * max_vals + j];
				float a = (*av)[i];
				float b = (*bv)[i * max_vals + j];
				operate(o, a, b, operation);
			}
		}
	}
	else if (b->val_type == V1 && a->val_type != V1)
	{
		// B as a scalar
		for (size_t i = 0; i < length; i++)
		{
			for (size_t j = 0; j < max_vals; j++)
			{
				float* o = &(*ov)[i * max_vals + j];
				float a = (*av)[i * max_vals + j];
				float b = (*bv)[i];
				operate(o, a, b, operation);
			}
		}
	}
	else
	{
		for (size_t i = 0; i < length * max_vals; i++)
		{
			float* o = &(*ov)[i];
			*o = 0.0f;
		}

		show_warning = true;
	}

	
}

void MathSPN::create(SurfaceProvider* surf)
{
	in_attribute[INPUT_A] = surf->create_attribute("A", id, true, ANY);
	in_attribute[INPUT_B] = surf->create_attribute("B", id, true, ANY);
	out_attribute[OUTPUT] = surf->create_attribute("R = ", id, false, ANY);

	this->surf = surf;

	val_a = glm::vec3(0.0f, 0.0f, 0.0f);
	val_b = glm::vec3(0.0f, 0.0f, 0.0f);
}
