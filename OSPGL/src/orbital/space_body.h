#pragma once
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/geometric.hpp>
#include "../util/defines.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef G
#define G (6.67 * std::pow(10, -11))
#endif

/*
	Represents the state of a thing moving using
	newtonian dynamics.
*/
struct NewtonState
{
	// Position of the body in 3D space
	glm::dvec3 pos;
	// Direction of the body (Normalized vector)
	glm::dvec3 dir;
	// Delta change (dir * vel)
	glm::dvec3 delta;
	// Velocity of the body. Must be m/s following IS
	double vel;

	// Previous position (Used in verlet integration)
	glm::dvec3 prev;

	void add_force(glm::dvec3 force)
	{
		delta += force;
		dir = glm::normalize(force);
		vel = glm::length(force);
	}

	double rotation;
};

/*
	Space Bodies are non-actively simulated (hence fixed),
	and are simulated according to the laws of gravitation, 
	using a 2-body problem solver

	Orbital properties are given as a set of parameters and not
	as a position and speed, like vessels are.

	A space body does not have to have a surface as such, but requires
	density. The rendering part of a space body is handled separately.

	All space_bodies will be handled by a superior class and hence they implement
	little actual functionality, just "data" functions.

	Reference plane is always given by the plane that intersects the equator
	of the parent body.

	We use SI units, so meters (m), kilograms (kg), seconds (s) and degrees (º)

	Read: https://en.wikipedia.org/wiki/Orbital_elements for info 
		about the math stuff
*/
class SpaceBody
{
public:

	std::string id;
	std::string parent_id; //< used by deserializer

	bool logged = false;

	SpaceBody* parent = NULL;

	// Used by the SpaceSystem to save a bunch of computations
	NewtonState last_state;
	NewtonState old_state;
	double old_true_anomaly;
	
	// Mass of the body, must be given in kg
	double mass;
	// Polar (north) vector of the planet. Used to get the equatorial plane
	glm::vec3 polar = glm::vec3(0, 1, 0);

	// Radius in meters 
	double radius;

	// Starting rotation in degrees at time 0
	double start_rotation;

	// Rotation alongside polar in seconds per revolution
	double rotation_speed;

	// Radius of the sphere of influence in meters
	double soi;

	// Orbit properties, only defined on bodies with a parent
	double eccentricity, smajor_axis, inclination, asc_node, arg_periapsis, true_anomaly;

	NewtonState to_state_at(double true_anom, double time, bool fast = true) const;
	NewtonState to_state(double time, bool fast = true) const;
	// Gets the state assuming the object has its parent at (0, 0, 0), used by the orbit drawer
	NewtonState to_state_origin(double true_anom) const;

	NewtonState state_from_mean(double mean) const;
	double mean_to_eccentric(double mean, double tol) const;
	double mean_to_true(double mean_anomaly, double tol = 1.0e-14) const;
	double time_to_mean(double time) const;
	double mean_to_time(double mean) const;
	double true_to_eccentric() const;
	double eccentric_to_mean(double eccentric) const;


	// Gets a point in the orbit given the time since epoch
	SpaceBody();
	~SpaceBody();
};

