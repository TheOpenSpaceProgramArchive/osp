#include "space_body.h"



glm::dvec3 space_body::pos_by_mean(double mean)
{
	double aeccentricity = std::abs(eccentricity);

	if (aeccentricity >= 1.0)
	{
		// Solve for this case too as escape trajectories SHOULD be possible

		if (!logged)
		{
			spdlog::get("OSP")->error("Eccentricity too high ({}). Can't solve! (TODO)", eccentricity);
			logged = true;
		}
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

double space_body::get_altitude(double t)
{
	double mean = get_mean_anomaly(t);
	double phi = get_true_anomaly(mean);

	return get_r_length(phi);

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
