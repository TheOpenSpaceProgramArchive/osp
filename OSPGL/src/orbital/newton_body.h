#pragma once

#include "space_body.h"
//#include "space_system.h"


class NewtonBody
{
public:

	enum SolverMethod
	{
		EULER,
		VERLET,
		RUNGE_KUTTA
	};

	NewtonState state;

	// loops determines how many times is the solve iterated, greater ammount 
	// of loops means higher precision, specially with EULER and VERLET methods.
	// dt is the delta-time which should not be very variable, try to use a fixed timestep
	//NewtonState solve(SpaceSystem system, SolverMethod method, int loops, float dt);

	//NewtonState solve_euler(SpaceSystem system, int loops, float dt);
	//NewtonState solve_verlet(SpaceSystem system, int loops, float dt);
	//NewtonState solve_runge_kutta(SpaceSystem system, int loops, float dt);


	NewtonBody();
	~NewtonBody();
};

