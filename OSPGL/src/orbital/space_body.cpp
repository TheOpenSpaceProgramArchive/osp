#include "space_body.h"

struct PosPack
{
	glm::dvec3 pos;
	double r;
};

static PosPack to_pos(double mass, SpaceBody* o)
{
	PosPack out;

	// Calculate radius (height)
	double r = (o->smajor_axis * (1.0 - (o->eccentricity * o->eccentricity))) / (1 + o->eccentricity * cos(o->true_anomaly));

	if (r < 0 && o->eccentricity >= 1.0)
	{
		return out;
	}

	// Adjust for other parameters

	out.pos.x = cos(o->true_anomaly) * r;
	out.pos.z = sin(o->true_anomaly) * r;

	// Arg.of.Periapsis
	out.pos = glm::rotateY(out.pos, glm::radians(o->arg_periapsis));
	// Inclination 
	out.pos = glm::rotateX(out.pos, glm::radians(o->inclination));
	// Ascending node
	out.pos = glm::rotateY(out.pos, glm::radians(o->asc_node));

	out.r = r;

	return out;
}




NewtonState SpaceBody::to_state(bool fast)
{
	NewtonState out;

	if (parent == NULL)
	{
		NewtonState st;
		st.pos = glm::vec3(0, 0, 0);
		return st;
	}
	// Sanity checks
	if (eccentricity < 0.0)
	{
		eccentricity = -eccentricity;
	}
	if (eccentricity >= 0.99999 && eccentricity <= 1.00001)
	{
		eccentricity += 0.01;
	}

	if (eccentricity < 1.0 && smajor_axis < 0)
	{
		smajor_axis = -smajor_axis;
	}

	if (eccentricity >= 1.0 && smajor_axis > 0)
	{
		smajor_axis = -smajor_axis;
	}



	PosPack cur = to_pos(mass + parent->mass, this);

	out.pos = cur.pos;
	if (!fast)
	{
		true_anomaly += 1e-11;
		glm::dvec3 next = to_pos(mass + parent->mass, this).pos;
		true_anomaly -= 1e-11;

		double vel = sqrt(G * (mass + parent->mass) * ((2.0 / cur.r) - (1.0 / smajor_axis)));

		out.dir = glm::normalize(next - out.pos);
		out.vel = vel;
		out.delta = out.dir * out.vel;
	}

	return out;
}

NewtonState SpaceBody::state_from_mean(double mean)
{
	return NewtonState();
}

// alpheratz.net/dynamics/twobody/KeplerIterations_summary.pdf
static double starting_value(double ecc, double mean)
{
	double t34 = ecc * ecc;
	double t35 = ecc * t34;
	double t33 = cos(mean);


	return mean + ((-1.0 / 2.0) * t35 + ecc + (t34 + (3.0 / 2.0) * t33 * t35) * t33) * sin(mean);
}

static double eps3(double ecc, double mean, double x)
{
	double t1 = cos(x);
	double t2 = -1 + ecc * t1;
	double t3 = sin(x);
	double t4 = ecc * t3;
	double t5 = -x + t4 + mean;
	double t6 = t5 / ((1.0 / 2.0) * t5 * (t4 / t2) + t2);
	return t5 / (((1.0 / 2.0) * t3 - (1.0 / 6.0) * t1 * t6) * ecc * t6 + t2);
}


double SpaceBody::true_to_eccentric()
{
	double upper = sqrt(1 - eccentricity * eccentricity) * sin(true_anomaly);
	double lower = eccentricity + cos(true_anomaly);

	return atan(upper / lower);
}

double SpaceBody::eccentric_to_mean(double eccentric)
{
	// Mean = eccentric - e * sin(eccentric), per the kepler equation
	return eccentric - eccentricity * sin(eccentric);
}

double SpaceBody::time_to_mean(double time)
{
	double sm = std::abs(smajor_axis);
	if (sm == 0)
	{
		return 0;
	}

	double n = sqrt((G * (mass + parent->mass)) / (sm * sm * sm));
	return n * time;
}

double SpaceBody::mean_to_time(double mean)
{
	double sm = std::abs(smajor_axis);
	if (sm == 0)
	{
		return 0;
	}

	double n = sqrt((G * (mass + parent->mass)) / (sm * sm * sm));

	// mean = n * time, time = n / mean
	return n / mean;
}

double SpaceBody::mean_to_eccentric(double mean, double tol)
{
	if (eccentricity < 1)
	{
		double out;

		double mnorm = fmod(mean, 2.0 * PI);
		double e0 = starting_value(eccentricity, mnorm);
		double de = tol + 1;
		double count = 0;
		while (de > tol)
		{
			out = e0 - eps3(eccentricity, mnorm, e0);
			de = abs(out - e0);
			e0 = out;
			count++;
			if (count >= 100)
			{
				return 0; // FATAL: Many iterations
			}
		}

		return out;
	}
	else
	{
		// mean = ecc * sinh(eccentric) - eccentric
		// SLOW SLOW, VERY SLOW method (for now):
		double step = 0.5;
		uint32_t count = 0;

		bool done = false;
		double r = 0;

		bool prev_inc = true;

		while (!done)
		{
			double rhs = eccentricity * sinh(r) - r;

			if (std::abs(mean - rhs) < 0.00001)
			{
				return r;
			}

			if (rhs < mean)
			{
				r += step;
				if (!prev_inc)
				{
					step /= 4.0;
				}
				prev_inc = true;
			}
			else
			{
				r -= step;
				if (prev_inc)
				{
					step /= 4.0;
				}
				prev_inc = false;
			}

			count++;

		}

		r = r;

		return r;

	}
}



double SpaceBody::mean_to_true(double mean_anomaly, double tol)
{
	double eccentric = mean_to_eccentric(mean_anomaly, tol);
	double half = eccentric / 2.0;

	if (eccentricity < 1.0)
	{
		return 2 * atan2(sqrt(1 + eccentricity) * sin(half), sqrt(1 - eccentricity) * cos(half));
	}
	else
	{
		return 2 * atan(sqrt((eccentricity + 1) / (eccentricity - 1)) * tanh(eccentric / 2.0));
	}
}

// TODO (Not working properly)
/*
double Orbit::from_state(EulerState state, double mass)
{

	glm::dvec3 hvec = glm::cross(state.pos, state.vel);
	double h = glm::length(hvec);
	double r = glm::length(state.pos);
	double v = glm::length(state.vel);
	double mu = G * mass;

	double energy = ((v * v) / 2.0) - (mu / r);

	smajor_axis = -(mu / (2.0 * energy));

	eccentricity = sqrt(1 - ((h * h) / (smajor_axis * mu)));

	inclination = std::abs(acos(hvec.y / h) - PI);

	//asc_node = std::abs(atan2(hvec.x, -hvec.z));
	double p = smajor_axis * (1.0 - eccentricity * eccentricity);

	true_anomaly = atan2(sqrt(p / mu) * glm::dot(state.pos, state.pos), p - r);

	double peri_plus_true = atan2(state.pos.y / sin(inclination), state.pos.x * cos(asc_node) + state.pos.z * sin(asc_node));
	//arg_of_periapsis = peri_plus_true - true_anomaly;

	return 0;
}
*/

SpaceBody::SpaceBody()
{
}


SpaceBody::~SpaceBody()
{
}
