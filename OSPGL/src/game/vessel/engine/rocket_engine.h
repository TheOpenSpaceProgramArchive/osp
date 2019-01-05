#pragma once
#include <cmath>

// The nozzle interpolates between throat_area
// and outleat_area over outlet_length
struct Nozzle
{
	float throat_area;
	float inlet_area;
	float inlet_slope;
	float inlet_length;
	float outlet_area;
	float throat_slope;
	float outlet_slope;
	float outlet_length;

	
	float get_area(float t);
};

class RocketEngine
{
public:

	RocketEngine();
	~RocketEngine();
};

