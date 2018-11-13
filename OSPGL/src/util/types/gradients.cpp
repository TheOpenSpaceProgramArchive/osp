#include "gradients.h"

static glm::vec4 interp(glm::vec4 p0, glm::vec4 p1, float t)
{
	return glm::vec4();
}

glm::vec4 Gradient::evaluate(float t)
{
	if (t < 0.0f || t > 1.0f)
	{
		abort(); // Invalid values given
	}

	float prev = 0.0f;
	glm::vec4 prev_val = keys[0.0f];

	for (auto it = keys.begin(); it != keys.end(); it++)
	{
		if (t > prev && t <= it->first)
		{
			// Interpolate between prev and first
			// prev is at 0 and it->first is at 1
			// so we substract prev and divide by (first - prev)
			float subval = (t - prev) / (it->first - prev);

			return interp(prev_val, it->second, subval);
		}

		prev = it->first;
		prev_val = it->second;
	}

	return glm::vec4(0, 0, 0, 0);
}