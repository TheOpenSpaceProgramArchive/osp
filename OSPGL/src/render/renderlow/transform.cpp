#include "transform.h"



glm::mat4 transform::build_matrix()
{
	mat = glm::mat4();

	// Operations are applied from bottom to top
	mat = glm::translate(mat, pos);
	mat = mat * glm::toMat4(rot);
	mat = glm::scale(mat, scl);

	return mat;
}

transform::transform()
{
}


transform::~transform()
{
}
