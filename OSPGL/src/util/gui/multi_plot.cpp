#include "multi_plot.h"



void MultiPlot::draw(size_t w, size_t h)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
	{
		return;
	}

	float y_scale = 0.0f;
	size_t max_count = 0;
	for (size_t i = 0; i < plots.size(); i++)
	{
		if (plots[i].abs_max > y_scale)
		{
			y_scale = plots[i].abs_max;
		}

		if (plots[i].data.size() > max_count)
		{
			max_count = plots[i].data.size();
		}
	}

	y_scale = 1.0f / y_scale;

	float x_scale = (float)w / (float)max_count;;


	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImRect frame_bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + w, window->DC.CursorPos.y + h));
	const ImRect inner_bb(frame_bb.Min, frame_bb.Max);
	const ImRect total_bb(frame_bb.Min, frame_bb.Max);

	ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

	// Draw base-lines
	window->DrawList->AddLine(ImVec2(frame_bb.Min.x, (inner_bb.Max.y + inner_bb.Min.y) / 2.0f),
		ImVec2(frame_bb.Min.x + (float)w, (inner_bb.Max.y + inner_bb.Min.y) / 2.0f),
		ImGui::GetColorU32(ImGuiCol_ButtonHovered));

	std::string max = std::to_string(1.0f / y_scale);
	ImVec2 t_size = ImGui::CalcTextSize(max.c_str());
	window->DrawList->AddText(ImVec2(frame_bb.Max.x - t_size.x, inner_bb.Min.y), ImGui::GetColorU32(ImGuiCol_ButtonHovered), max.c_str());
	std::string min = std::to_string(-1.0f / y_scale);
	t_size = ImGui::CalcTextSize(min.c_str());
	window->DrawList->AddText(ImVec2(frame_bb.Max.x - t_size.x, inner_bb.Max.y - t_size.y), ImGui::GetColorU32(ImGuiCol_ButtonHovered), min.c_str());

	for (size_t i = 0; i < plots.size(); i++)
	{
		if (plots[i].data.size() > 0)
		{
			ImColor color = ImColor(plots[i].color.x, plots[i].color.y, plots[i].color.z);
			float x = 0.0f;
			float prev_y = (inner_bb.Max.y + inner_bb.Min.y) / 2.0f;
			for (size_t j = 0; j < plots[i].data.size(); j++)
			{
				float y = plots[i].data[j];
				y *= y_scale;
				y *= (float)h / 2.0f;
				float real_x = inner_bb.Min.x + x;
				float real_y = (inner_bb.Max.y + inner_bb.Min.y) / 2.0f - y;

				window->DrawList->AddLine(ImVec2(real_x - x_scale, prev_y), ImVec2(real_x, real_y), color);
				x += x_scale;
				prev_y = real_y;
			}

			std::string label = plots[i].label;
			label += " -> (";
			label += std::to_string(plots[i].data[plots[i].data.size() - 1]);
			label += ")";

			// Draw labels
			window->DrawList->AddText(ImVec2(inner_bb.Max.x, prev_y), color, label.c_str());
			
		}

	}

}

void MultiPlot::add_data(std::string label, float data)
{
	for (size_t i = 0; i < plots.size(); i++)
	{
		if (plots[i].label == label)
		{
			plots[i].data.push_back(data);
			if (std::abs(data) > plots[i].abs_max)
			{
				plots[i].abs_max = std::abs(data);
			}
			return;
		}
	}
}

void MultiPlot::create_plot(std::string label, glm::vec3 color)
{
	PlotData plot;
	plot.label = label;
	plot.color = color;
	plots.push_back(plot);
}

MultiPlot::MultiPlot()
{
}


MultiPlot::~MultiPlot()
{
}
