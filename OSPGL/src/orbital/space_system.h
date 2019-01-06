#pragma once

#include "space_body.h"
#include "newton_body.h"
#include <imgui/imgui.h>
#include "../util/gui/multi_plot.h"
#include "../render/renderlow/debug_draw.h"
#include <sstream>
#ifndef G
#define G (6.67 * std::pow(10, -11))
#endif

struct RK4Derivative
{
	glm::dvec3 pos;
	glm::dvec3 delta;

	RK4Derivative operator+(RK4Derivative b)
	{
		RK4Derivative out;
		out.pos = pos + b.pos; out.delta = delta + b.delta;
		return out;
	}
	void operator+=(RK4Derivative b)
	{
		pos += b.pos; delta += b.delta;
	}
	RK4Derivative operator*(double b)
	{
		RK4Derivative out;
		out.pos = pos * b; out.delta = delta * b;
		return out;
	}
	void operator*=(double b)
	{
		pos *= b; delta *= b;
	}

	RK4Derivative(glm::dvec3 pos, glm::dvec3 delta)
	{
		this->pos; this->delta = delta;
	}

	RK4Derivative()
	{

	}
};

class SpaceSystem
{
public:

	// Time since epoch in seconds
	double time;

	std::vector<SpaceBody*> bodies;
	std::vector<NewtonBody*> newton_bodies;

	MultiPlot plot;

	// Computes instantaneous gravitational force at a given point
	// Mass of second object is assumed to be extremely small compared to the
	// planetary body, but we could implement it too.
	glm::dvec3 compute_force(glm::dvec3 pos);

	glm::dvec3 compute_force_at(glm::dvec3 pos, double t0);

	// timewarp is the ammount of seconds we fit in one real second,
	// dt is the ammount of seconds that passes since last call to simulate
	void simulate(float timewarp, float dt, double* t, NewtonBody::SolverMethod method);

	// Propagates a newton state
	// Always uses velocity verlet because it's the most appropiate method for this
	// kind of operation.
	NewtonState simulate_static(double dt, double t0, NewtonState st);

	// Can't throw as there are no fatal errors, invalid data is discarded
	void deserialize(std::string data);

	void draw_debug_data(DebugDraw* debug, double scale);

	// Always works
	std::string serialize();

	// Gets the IDs of every thing in the system
	std::vector<std::string> get_all_ids();
	// Gets the IDs of every planetary body in the system
	std::vector<std::string> get_kepler_ids();
	// Gets the IDs of every vessel (newton body) in the system
	std::vector<std::string> get_newton_ids();

	SpaceBody* find_body(std::string id) const;

	NewtonBody* find_newton(std::string id) const;

	// Finds the position of a body or newton body
	// given id. Prioritized planetary bodies
	glm::dvec3 find_pos(std::string id, bool no_error = false) const;

	SpaceSystem();
	~SpaceSystem();
};

