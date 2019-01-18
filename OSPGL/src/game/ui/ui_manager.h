#pragma once
#include <unordered_map>
#include <imgui/imgui.h>
#include <time.h>
#include <string>

// Simple class to unify central UI drawing
class UIManager
{
public:

	bool orbit_focus = true;
	bool orbit_plotter;
	bool time_control = true;

	bool de_laval_nozzle;

	float timewarp;
	float prev_timewarp;
	double time;

	void draw();

};

