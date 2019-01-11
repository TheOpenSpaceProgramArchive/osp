#include "dsprite.h"

Shader DSprite::SPRITE_SHADER = Shader();

void DSprite::draw(glm::mat4 view, glm::mat4 proj)
{
	DSprite::SPRITE_SHADER.use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.texture);

	DSprite::SPRITE_SHADER.seti("tex", 0);
	DSprite::SPRITE_SHADER.setmat4("model", tform.build_matrix());
	DSprite::SPRITE_SHADER.setmat4("view", view);
	DSprite::SPRITE_SHADER.setmat4("proj", proj);

	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_count);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

}

static void generate_mesh(Mesh* mesh)
{

	Vertex tl, tr, bl, br;
	tl.pos = glm::vec3(-1, -1, 0); tl.uv = glm::vec2(0, 1);
	tr.pos = glm::vec3(1, -1, 0); tr.uv = glm::vec2(1, 1);
	bl.pos = glm::vec3(-1, 1, 0); bl.uv = glm::vec2(0, 0);
	br.pos = glm::vec3(1, 1, 0), br.uv = glm::vec2(1, 0);
	// Create framebuffers for rendering


	mesh->vertices.push_back(tl);
	mesh->vertices.push_back(tr);
	mesh->vertices.push_back(bl);
	mesh->vertices.push_back(tr);
	mesh->vertices.push_back(br);
	mesh->vertices.push_back(bl);

	mesh->build_array();
	mesh->upload();

}

DSprite::DSprite(GLuint tex) : texture(tex)
{
	generate_mesh(&mesh);
}

DSprite::DSprite(std::string texture_path) : texture(texture_path)
{
	if (SPRITE_SHADER.program == 0)
	{
		SPRITE_SHADER = Shader("res/shaders/generic/sprite.vs", "res/shaders/generic/sprite.fs");
	}

	generate_mesh(&mesh);
}

DSprite::~DSprite()
{
}
