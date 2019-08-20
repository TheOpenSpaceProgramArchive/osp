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
	float old_a = val_a;
	float old_b = val_b;

	ImGui::PushItemWidth(60.0f);
	if (id == INPUT_A)
	{
		if (in_attribute[INPUT_A]->links.size() == 0)
		{
			in_attribute[INPUT_A]->has_values = true;

			ImGui::InputFloat("", &val_a);
		}
		else
		{
			in_attribute[INPUT_A]->has_values = false;
		}
	}
	else if (id == INPUT_B)
	{
		if (in_attribute[INPUT_B]->links.size() == 0)
		{
			in_attribute[INPUT_B]->has_values = true;

			ImGui::InputFloat("", &val_b);
		}
		else
		{
			in_attribute[INPUT_B]->has_values = false;
		}
	}
	ImGui::PopItemWidth();



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
		ImGui::PopItemWidth();

		if (operation != old_op)
		{
			return true;
		}
	}

	if (val_a != old_a || val_b != old_b)
	{
		return true;
	}

	return false;
}

std::string MathSPN::get_name()
{
	return "Math";
}

void MathSPN::process(size_t length)
{
	std::vector<float>* target = &out_attribute[OUTPUT]->values;
	target->resize(length);

	if (in_attribute[INPUT_A]->has_values)
	{
		in_attribute[INPUT_A]->values.resize(length);
		std::fill_n(in_attribute[INPUT_A]->values.begin(), length, val_a);
	}

	if (in_attribute[INPUT_B]->has_values)
	{
		in_attribute[INPUT_B]->values.resize(length);
		std::fill_n(in_attribute[INPUT_B]->values.begin(), length, val_b);
	}

	std::vector<float>* a = &in_attribute[INPUT_A]->values;
	std::vector<float>* b = &in_attribute[INPUT_B]->values;

	for (size_t i = 0; i < length; i++)
	{
		if (operation == MathSPN::ADD)
		{
			(*target)[i] = (*a)[i] + (*b)[i];
		}
		else if (operation == MathSPN::SUBSTRACT)
		{
			(*target)[i] = (*a)[i] - (*b)[i];
		}
		else if (operation == MathSPN::MULTIPLY)
		{
			(*target)[i] = (*a)[i] * (*b)[i];
		}
		else if (operation == MathSPN::DIVIDE)
		{
			(*target)[i] = (*a)[i] / (*b)[i];
		}
		else if (operation == MathSPN::MODULO)
		{
			(*target)[i] = fmod((*a)[i], (*b)[i]);
		}
		else if (operation == MathSPN::POWER)
		{
			(*target)[i] = pow((*a)[i], (*b)[i]);
		}
	}
}

void MathSPN::create(SurfaceProvider* surf)
{
	in_attribute[INPUT_A] = surf->create_attribute("A", id, true);
	in_attribute[INPUT_B] = surf->create_attribute("B", id, true);
	out_attribute[OUTPUT] = surf->create_attribute("R = ", id, false);

	val_a = 0.0f;
	val_b = 0.0f;
}
