#include "space_body.h"



glm::dvec3 space_body::pos_by_anomaly(double anomaly)
{
	return glm::dvec3();
}

glm::dvec3 space_body::pos_by_time(double t)
{
	double mean = get_mean_anomaly(t);
	double ecc = get_eccentric_anomaly(mean);
	double phi = get_true_anomaly(mean);
	double r = get_r_length(phi);

	glm::dvec3 out;
	out.x = r * cos(phi);
	out.y = r * sin(phi);

	return out;
}

double space_body::get_orbital_period()
{
	if (parent)
	{
		return 2 * PI * std::sqrt(std::pow(smajor_axis, 3) / (G * parent->mass));
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
		return smajor_axis * (1 - eccentricity);
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
		return smajor_axis * (1 + eccentricity);
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

		temp_result = numerator / (1 - eccentricity * cos(temp_result));
	}

	return temp_result;
}

double space_body::get_mean_anomaly(double t)
{
	return ((2 * PI) / get_orbital_period()) * t;
}

double space_body::get_true_anomaly(double mean_anomaly, int iterations)
{
	return 2 * atan(sqrt((1 + eccentricity) / (1 - eccentricity)) * tan(get_eccentric_anomaly(mean_anomaly, iterations) / 2));
}

double space_body::get_r_length(double true_anomaly)
{
	double nominator = smajor_axis * 2 * (1 - pow(eccentricity, 2));
	double denominator = 1 + eccentricity * cos(true_anomaly);

	return nominator / denominator;
}



space_body::space_body()
{
}


space_body::~space_body()
{
}
