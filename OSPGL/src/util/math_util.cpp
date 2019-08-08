#include "math_util.h"

// http://mathproofs.blogspot.com/2005/07/mapping-cube-to-sphere.html
glm::vec3 MathUtil::cube_to_sphere(glm::vec3 cubic)
{
	glm::vec3 out;

	float xsq = cubic.x * cubic.x;
	float ysq = cubic.y * cubic.y;
	float zsq = cubic.z * cubic.z;

	out.x = cubic.x * sqrtf(1.0f - (ysq / 2.0f) - (zsq / 2.0f) + ((ysq * zsq) / 3.0f));
	out.y = cubic.y * sqrtf(1.0f - (xsq / 2.0f) - (zsq / 2.0f) + ((xsq * zsq) / 3.0f));
	out.z = cubic.z * sqrtf(1.0f - (xsq / 2.0f) - (ysq / 2.0f) + ((xsq * ysq) / 3.0f));

	return out;
}
