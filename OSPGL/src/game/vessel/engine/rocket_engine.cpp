#include "rocket_engine.h"



RocketEngine::RocketEngine()
{
}


RocketEngine::~RocketEngine()
{
}

float Nozzle::get_area(float t)
{
	if (t <= inlet_length)
	{
		float x = t / inlet_length;

		float t0 = (std::powf(x, 3.0f) - 2.0f * std::powf(x, 2.0f) + x);
		float t1 = (2.0f * std::powf(x, 3.0f) - 3.0f * std::powf(x, 2.0f) + 1.0f);
		float t2 = (-2.0f * std::powf(x, 3.0f) + 3.0f * std::powf(x, 2.0f));
		return t0 * inlet_slope + t1 * inlet_area + t2 * throat_slope;
	}
	else
	{
		float x = t / outlet_length;

		float t0 = (std::powf(x, 3.0f) - 2.0f * std::powf(x, 2.0f) + x);
		float t1 = (std::powf(x, 3.0f) - std::powf(x, 2.0f));
		float t2 = (2.0f * std::powf(x, 3.0f) - 3.0f * std::powf(x, 2.0f) + 1.0f);
		float t3 = (-2.0f * std::powf(x, 3.0f) + 3.0f * std::powf(x, 2.0f));

		return t0 * throat_slope + t1 * outlet_slope + t2 * throat_area + t3 * outlet_area;
	}
}
