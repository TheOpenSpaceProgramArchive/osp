#pragma once
#include "../drawable.h"
#include "../shader.h"
#include "../mesh.h"
#include "../texture.h"


class DSprite : public Drawable
{
private:

	Mesh mesh;


public:

	Texture texture;

	static Shader SPRITE_SHADER;

	Transform tform;

	virtual void draw(glm::mat4 view, glm::mat4 proj) override;

	DSprite(GLuint tex);
	DSprite(std::string texture_path);
	~DSprite();
};

