#include "transform.h"



glm::mat4 Transform::build_matrix()
{
	glm::mat4 parent_mat;
	if (parent != NULL)
	{
		parent_mat = parent->build_matrix();
	}

	mat = glm::mat4();

	// Operations are applied from bottom to top
	mat = mat * parent_mat;
	mat = glm::translate(mat, pos);
	mat = mat * glm::toMat4(rot);
	mat = glm::scale(mat, scl);

	return mat;
}

Transform::Transform()
{
}


Transform::~Transform()
{
}
