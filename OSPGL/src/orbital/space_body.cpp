#include "space_body.h"


float mag(glm::dvec3 vec)
{
	return std::sqrt(std::pow(vec.x, 2) + std::pow(vec.y, 2) + std::pow(vec.z, 2));
}

newton_state space_body::state_by_mean(double anomaly, double precision)
{
	newton_state out;

	out.pos = pos_by_mean(anomaly);
	out.dir = dir_by_mean(anomaly);
	out.vel = vel_by_radius(get_altitude_mean(anomaly));
	out.delta = out.vel * out.dir;

	return out;
}

newton_state space_body::state_by_time(double t, double precision)
{
	double mean = get_mean_anomaly(t);

	return state_by_mean(mean, precision);
}

void space_body::set_state(newton_state state)
{

	// Decent method but is pretty innacurate on extreme cases

	state.pos /= 2;

	double mu = parent->mass * G;

	double rad = mag(state.pos);
	double vel = state.vel;

	
	// Specific Angular momentum (h)
	glm::dvec3 ang_momentum_v = glm::cross(state.pos, state.delta);

	double ang_momentum = mag(ang_momentum_v);

	// Specific Energy
	double s_energy = (std::pow(vel, 2) / 2.0) - (mu / rad);

	// Semi Major Axis
	smajor_axis = -mu / (2.0 * s_energy);

	// Eccentricity
	eccentricity = std::sqrt(1.0 - (std::pow(ang_momentum, 2) / (smajor_axis * mu)));

	// Inclination (As it's given in 0->180 we have to substract it from 180)
	inclination = 180 - glm::degrees(acos(ang_momentum_v.y / ang_momentum));

	// Asc Node
	asc_node = glm::degrees(atan2(ang_momentum_v.x, -ang_momentum_v.z));
}

glm::dvec3 space_body::pos_by_mean(double mean)
{
	double aeccentricity = std::abs(eccentricity);

	if (aeccentricity >= 1.0)
	{
		// Solve for this case too as escape trajectories SHOULD be possible

		double semi_latus = smajor_axis * (1 - std::pow(eccentricity, 2));
		double true_anomaly = get_true_anomaly_hyperbolic(mean);
		double radius = semi_latus / (1 + eccentricity * cos(true_anomaly));

		return { 0, 0, 0 };
	}
	else
	{
		double ecc = get_eccentric_anomaly(mean);

		int iterations = 4;

		if (aeccentricity > 0.8 && aeccentricity < 0.92)
		{
			iterations = 8;
		}
		else if (aeccentricity >= 0.92 && aeccentricity < 0.95)
		{
			iterations = 24;
		}
		else if (aeccentricity >= 0.95 && aeccentricity < 0.98)
		{
			iterations = 48;
		}
		else if (aeccentricity >= 0.98)
		{
			iterations = 200;
		}

		double phi = get_true_anomaly(mean, iterations);
		double r = get_r_length(phi);

		glm::dvec3 out;
		out.x = r * cos(phi);
		out.z = r * sin(phi);


		// Rotate to handle the argument of periapsis
		// Basically a rotation along the y axis

		out = glm::rotateY(out, glm::radians(arg_periapsis));


		/*	Now rotate to fit inclination and long_asc_node
			Inclination is simply applied by rotating 'inclination' degrees
			the orbit, given the longitude of the ascending node as the
			axis of rotation
		 */

		// First lets get the rotation axis. It's simply the x-z vector as
		// specified by the asc_node angle (We have to convert to radians)
		glm::dvec3 rot;
		rot.x = cos(glm::radians(asc_node));
		rot.z = sin(glm::radians(asc_node));

		// Now rotate with help from glm

		out = glm::rotate(out, glm::radians(inclination), rot);


		return out;
	}
}

glm::dvec3 space_body::pos_by_time(double t)
{
	double mean = get_mean_anomaly(t);
	
	return pos_by_mean(mean);
}

glm::dvec3 space_body::dir_by_mean(double mean, double precision)
{
	glm::dvec3 pos_a = pos_by_mean(mean);
	glm::dvec3 pos_b = pos_by_mean(mean + precision);
	
	glm::dvec3 out = pos_b - pos_a;
	out = glm::normalize(out);

	return out;
}

double space_body::vel_by_radius(double r)
{
	double v_sq = G * parent->mass * ((2.0 / r) - (1.0 / smajor_axis));
	return std::sqrt(v_sq);
}

double space_body::get_orbital_period()
{
	if (parent)
	{
		return 2.0 * PI * std::sqrt(std::pow(smajor_axis, 3) / (G * parent->mass));
	}
	else
	{
		return INFINITY;
	}
}

double space_body::get_periapsis_radius()
{
	if (parent)
	{
		return smajor_axis * (1.0 - eccentricity);
	}
	else
	{
		return 0;
	}
}

double space_body::get_apoapsis_radius()
{
	if (parent)
	{
		return smajor_axis * (1.0 + eccentricity);
	}
	else
	{
		return 0;
	}
}


double space_body::get_eccentric_anomaly(double mean_anomaly, int iterations)
{
	double temp_result = mean_anomaly;

	double numerator;

	for (int i = 0; i < iterations; i++)
	{
		numerator = mean_anomaly + eccentricity * sin(temp_result)
			- eccentricity * temp_result * cos(temp_result);

		temp_result = numerator / (1.0 - eccentricity * cos(temp_result));
	}

	return temp_result;
}

double space_body::get_mean_anomaly(double t)
{
	return ((2.0 * PI) / get_orbital_period()) * t;
}


double space_body::get_true_anomaly(double mean_anomaly, int iterations)
{
	return 2.0 * atan(sqrt((1.0 + eccentricity) / (1.0 - eccentricity)) * tan(get_eccentric_anomaly(mean_anomaly, iterations) / 2.0));
}

double space_body::get_true_anomaly_hyperbolic(double mean_anomaly, int iterations)
{
	return 0.0;
}

double space_body::get_altitude(double t)
{
	double mean = get_mean_anomaly(t);
	return get_altitude_mean(mean);
}

double space_body::get_altitude_mean(double mean)
{
	double phi = get_true_anomaly(mean);

	return get_r_length(phi) / 2.0;
}

double space_body::get_r_length(double true_anomaly)
{
	double nominator = smajor_axis * 2.0 * (1.0 - pow(eccentricity, 2));
	double denominator = 1.0 + eccentricity * cos(true_anomaly);

	return nominator / denominator;
}



space_body::space_body()
{
}


space_body::~space_body()
{
}
