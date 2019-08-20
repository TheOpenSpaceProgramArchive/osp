#include "noise_spn.h"

#include "../surface_provider.h"

std::string noise_type_to_str(NoiseSPN::NoiseType type)
{
	if (type == NoiseSPN::PERLIN)
	{
		return "Perlin";
	}
	else if (type == NoiseSPN::PERLIN_SEED)
	{
		return "Seeded Perlin";
	}
	else if (type == NoiseSPN::RIDGE)
	{
		return "Ridge Fractal";
	}
	else if (type == NoiseSPN::FBM)
	{
		return "Fractal Brownian Motion";
	}
	else if (type == NoiseSPN::TURBULENCE)
	{
		return "Turbulence Fractal";
	}
	else
	{
		return "Unknown";
	}
}

bool NoiseSPN::do_imgui(int id)
{
	bool needs_dirty = false;

	ImGui::PushItemWidth(80.0f);
	if (id == INPUT_X)
	{
		ImGui::SameLine();
		needs_dirty |= ImGui::InputFloat("Scale", &in_scale);
	}
	else if (id == INPUT_LACUNARITY)
	{
		if (noise_type == PERLIN || noise_type == PERLIN_SEED)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("(Disabled)");
			in_attribute[INPUT_LACUNARITY]->has_values = true;
		}
		else
		{
			if (in_attribute[INPUT_LACUNARITY]->links.size() == 0)
			{
				in_attribute[INPUT_LACUNARITY]->has_values = true;
				ImGui::SameLine();
				needs_dirty |= ImGui::InputFloat("", &val_lacunarity);
			}
			else
			{
				in_attribute[INPUT_LACUNARITY]->has_values = false;
			}
		}
	}
	else if (id == INPUT_GAIN)
	{
		if (noise_type == PERLIN || noise_type == PERLIN_SEED)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("(Disabled)");
			in_attribute[INPUT_GAIN]->has_values = true;
		}
		else
		{
			if (in_attribute[INPUT_GAIN]->links.size() == 0)
			{
				in_attribute[INPUT_GAIN]->has_values = true;
				ImGui::SameLine();
				needs_dirty |= ImGui::InputFloat("", &val_gain);
			}
			else
			{
				in_attribute[INPUT_GAIN]->has_values = false;
			}
		}
	}
	else if (id == INPUT_OFFSET)
	{
		if (noise_type != RIDGE)
		{
			ImGui::SameLine();
			ImGui::TextDisabled("(Disabled)");
			in_attribute[INPUT_OFFSET]->has_values = true;
		}
		else
		{

			if (in_attribute[INPUT_OFFSET]->links.size() == 0)
			{
				in_attribute[INPUT_OFFSET]->has_values = true;
				ImGui::SameLine();
				needs_dirty |= ImGui::InputFloat("", &val_offset);
			}
			else
			{
				in_attribute[INPUT_OFFSET]->has_values = false;
			}
		}
	}
	else if (id == OUTPUT) 
	{
		needs_dirty |= ImGui::InputInt("Octaves", &octaves);
		
		if (noise_type == NoiseType::PERLIN_SEED)
		{
			needs_dirty |= ImGui::InputInt("Seed", &seed);
		}

		if (noise_type == NoiseType::PERLIN || noise_type == NoiseType::PERLIN_SEED)
		{
			needs_dirty |= ImGui::InputInt3("Wrap (2^x)", wrap_pow);

			if (needs_dirty)
			{
				for (size_t i = 0; i < 3; i++)
				{
					if (wrap_pow[i] < -1)
					{
						wrap_pow[i] = -1;
						wrap[i] = 0;
					}
					else
					{
						wrap[i] = pow(2, wrap_pow[i]);
					}
				}
			}
		}
		
		needs_dirty |= ImGui::InputFloat("Scale", &out_scale);

		NoiseType old = noise_type;

		std::string noise_str = noise_type_to_str(noise_type);
		ImGui::PopItemWidth();
		ImGui::PushItemWidth(120.0f);
		if (ImGui::BeginCombo("", noise_str.c_str()))
		{
			for (int i = 0; i < NoiseType::END_MARKER; i++)
			{
				if (ImGui::Selectable(noise_type_to_str(NoiseType(i)).c_str()))
				{
					noise_type = NoiseType(i);
				}
			}

			ImGui::EndCombo();
		}

		if (old != noise_type)
		{
			needs_dirty = true;
		}

	
	}

	ImGui::PopItemWidth();


	return needs_dirty;
}

std::string NoiseSPN::get_name()
{
	return "Noise";
}

void NoiseSPN::process(size_t length)
{
	std::vector<float>* out = &out_attribute[OUTPUT]->values;
	out->resize(length, 0.0f);

	if (in_attribute[INPUT_LACUNARITY]->has_values)
	{
		in_attribute[INPUT_LACUNARITY]->values.resize(length);
		std::fill_n(in_attribute[INPUT_LACUNARITY]->values.begin(), length, val_lacunarity);
	}

	if (in_attribute[INPUT_GAIN]->has_values)
	{
		in_attribute[INPUT_GAIN]->values.resize(length);
		std::fill_n(in_attribute[INPUT_GAIN]->values.begin(), length, val_gain);
	}

	if (in_attribute[INPUT_OFFSET]->has_values)
	{
		in_attribute[INPUT_OFFSET]->values.resize(length);
		std::fill_n(in_attribute[INPUT_OFFSET]->values.begin(), length, val_offset);
	}

	for (size_t i = 0; i < length; i++)
	{
		float x = in_attribute[INPUT_X]->values[i * 3 + 0] * in_scale;
		float y = in_attribute[INPUT_X]->values[i * 3 + 1] * in_scale;
		float z = in_attribute[INPUT_X]->values[i * 3 + 2] * in_scale;
		float lacunarity = in_attribute[INPUT_LACUNARITY]->values[i];
		float gain = in_attribute[INPUT_GAIN]->values[i];
		float offset = in_attribute[INPUT_OFFSET]->values[i];

		if (noise_type == PERLIN)
		{
			(*out)[i] = stb_perlin_noise3(x, y, z, wrap[0], wrap[1], wrap[2]) * out_scale;
		}
		else if (noise_type == PERLIN_SEED)
		{
			(*out)[i] = stb_perlin_noise3_seed(x, y, z, wrap[0], wrap[1], wrap[2], seed) * out_scale;
		}
		else if (noise_type == RIDGE)
		{
			(*out)[i] = stb_perlin_ridge_noise3(x, y, z, lacunarity, gain, offset, octaves) * out_scale;
		}
		else if (noise_type == FBM)
		{
			(*out)[i] = stb_perlin_fbm_noise3(x, y, z, lacunarity, gain, octaves) * out_scale;
		}
		else if (noise_type == TURBULENCE)
		{
			(*out)[i] = stb_perlin_turbulence_noise3(x, y, z, lacunarity, gain, octaves) * out_scale;
		}
	}
}

void NoiseSPN::create(SurfaceProvider* surf)
{
	in_attribute[INPUT_X] = surf->create_attribute("Position", id, true, V3);
	in_attribute[INPUT_LACUNARITY] = surf->create_attribute("Lacunarity", id, true, V1);
	in_attribute[INPUT_GAIN] = surf->create_attribute("Gain", id, true, V1);
	in_attribute[INPUT_OFFSET] = surf->create_attribute("Offset", id, true, V1);

	out_attribute[OUTPUT] = surf->create_attribute("Out", id, false, V1);

	in_scale = 1.0f;

	val_offset = 1.0f;
	val_lacunarity = 2.0f;
	val_gain = 0.5f;
	octaves = 6;

	wrap[0] = 0;
	wrap[1] = 0;
	wrap[2] = 0;
	wrap_pow[0] = -1;
	wrap_pow[1] = -1;
	wrap_pow[2] = -1;
	out_scale = 1.0f;

	noise_type = PERLIN;

}


