#pragma once
#include <cmath>
#include <vector>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

struct NozzleData
{
	float radius;
	float mach;
	float temp;
	float pres;
};

// The nozzle interpolates between throat_area
// and outleat_area over outlet_length
struct Nozzle
{
	float throat_radius;
	float inlet_radius;
	float inlet_slope;
	float inlet_length;
	float outlet_radius;
	float throat_slope;
	float outlet_slope;
	float outlet_length;

	
	float get_radius(float t);
	// p0 = Pressure at inlet
	// t0 = Temperature at inlet
	// adiabatic = Adiabatic index of the gases
	// t = location on the nozzle
	NozzleData simulate(float p0, float t0, float adiabatic, float t);
};

class RocketEngine
{
private:
	int item = 0;
public:

	float propellant_k = 1.8f;
	float inlet_t = 3000.0f;
	float inlet_p = 2.0f;


	Nozzle nozzle;

	void imgui_draw();

	RocketEngine();
	~RocketEngine();
};

