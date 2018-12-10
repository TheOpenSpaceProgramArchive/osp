#pragma once
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

struct PlotData
{
	std::string label;
	glm::vec3 color;
	float abs_max;
	std::vector<float> data;
};

class MultiPlot
{
public:

	std::vector<PlotData> plots;

	// Sends the IMGUI commands
	void draw(size_t w, size_t h);

	// If label is not present it does nothing
	void add_data(std::string label, float data);
	void create_plot(std::string label, glm::vec3 color);

	MultiPlot();
	~MultiPlot();
};

