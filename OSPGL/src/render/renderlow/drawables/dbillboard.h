#pragma once
#include "../drawable.h"
#include "../mesh.h"
#include "../shader.h"
#include "../transform.h"

class dbillboard : public drawable
{
public:

	transform tform;

	mesh square;
	shader* shader;

	virtual void draw(glm::mat4 view, glm::mat4 proj);

	dbillboard();
	~dbillboard();
};

