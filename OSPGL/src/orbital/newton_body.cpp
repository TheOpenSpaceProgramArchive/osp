#include "newton_body.h"



void newton_body::step(double dt, int samples)
{
	if (samples <= 0)
	{
		// calculate subsamples
	}
	else
	{
		double step_dt = dt / samples;
		// Step
		for (int i = 0; i < samples; i++)
		{
			substep(step_dt);
		}
	}
}

void newton_body::substep(double dt)
{
}

newton_body::newton_body()
{
}


newton_body::~newton_body()
{
}
