#include "newton_body.h"



NewtonState NewtonBody::solve(SpaceSystem system, SolverMethod method, int loops, float dt)
{
	if (method == SolverMethod::EULER)
	{
		return solve_euler(system, loops, dt);
	}
	else if (method == SolverMethod::VERLET)
	{
		return solve_verlet(system, loops, dt);
	}
	else
	{
		return solve_runge_kutta(system, loops, dt);
	}
}

NewtonState NewtonBody::solve_euler(SpaceSystem system, int loops, float dt)
{
	double realDelta = dt / loops;
	NewtonState copy = state;
	for (size_t i = 0; i < loops; i++)
	{
		glm::dvec3 force = system.computeForce(state.pos) * realDelta;
		copy.add_force(force);
	}

	return copy;
}

NewtonState NewtonBody::solve_verlet(SpaceSystem system, int loops, float dt)
{
	return NewtonState();
}

NewtonState NewtonBody::solve_runge_kutta(SpaceSystem system, int loops, float dt)
{
	return NewtonState();
}

NewtonBody::NewtonBody()
{
}


NewtonBody::~NewtonBody()
{
}
