#pragma once
#include <unordered_map>
#include <imgui/imgui.h>

// Simple class to unify central UI drawing
class UIManager
{
public:

	bool orbit_focus = true;
	bool orbit_plotter;
	bool time_control = true;

	bool de_laval_nozzle;
	float time;
	float true_anom;
	float period;

	void draw();

};

