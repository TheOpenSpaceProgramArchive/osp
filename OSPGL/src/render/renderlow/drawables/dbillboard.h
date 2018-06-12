#pragma once
#include "../drawable.h"
#include "../mesh.h"
#include "../shader.h"
#include "../transform.h"

class DBillboard : public Drawable
{
public:

	Transform tform;

	Mesh square;
	Shader* Shader;

	virtual void draw(glm::mat4 view, glm::mat4 proj);

	DBillboard();
	~DBillboard();
};

