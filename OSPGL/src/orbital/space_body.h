#pragma once
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

#ifndef G
#define G 0.000000000066742 
#endif

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
class space_body
{
public:

	bool logged = false;

	space_body* parent = NULL;
	
	// Mass of the body, must be given in kg
	double mass;
	// Polar (north) vector of the planet. Used to get the equatorial plane
	glm::dvec3 polar;

	// Orbit properties, only defined on bodies with a parent
	double eccentricity, smajor_axis, inclination, asc_node, arg_periapsis;

	// The given argument is the angular distance from the pericenter if the body
	// was in a circular orbit of same period as the actual orbit.
	// 360 is a full orbit
	// (Takes mean anomaly)
	glm::dvec3 pos_by_mean(double anomaly);

	// t is the time in seconds since passing the periapsis
	glm::dvec3 pos_by_time(double t);

	// Obtains the time it takes for a full orbit to be completed
	double get_orbital_period();

	double get_periapsis_radius();

	double get_apoapsis_radius();

	// Obtains eccentric anomaly using recursive method
	// 4 iterations seem good enough
	double get_eccentric_anomaly(double mean_anomaly, int iterations = 4);

	// Gets mean anomaly given time
	double get_mean_anomaly(double t);

	// Gets true anomaly given mean_anomaly using get_eccentric_anomaly
	double get_true_anomaly(double mean_anomaly, int iterations = 4);

	// Gets altitude given time
	double get_altitude(double t);

	double get_r_length(double true_anomaly);

	// Gets a point in the orbit given the time since epoch
	space_body();
	~space_body();
};

