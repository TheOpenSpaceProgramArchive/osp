#pragma once

#include "drawable.h"

#include <vector>
#include <glm/glm.hpp>
#include "shader.h"

struct DebugDrawCommand
{
	glm::vec4 color;
	GLuint draw_mode;
	float time;
	GLuint vao, vbo;
	float point_size;

	void generate(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2);
};

class DebugDraw : Drawable
{
public:

	Shader* shader;

	std::vector<DebugDrawCommand> commands;

	void add_point(glm::vec3 pos, glm::vec4 color, float point_size = 3.0f, float time = 0.0f);
	void add_line(glm::vec3 pos0, glm::vec3 pos1, glm::vec4 color, float time = 0.0f);
	void add_arrow(glm::vec3 pos0, glm::vec3 pos1, glm::vec4 color, float time = 0.0f);
	void add_cross(glm::vec3 pos, float radius, glm::vec4 color, float time = 0.0f);

	virtual void draw(glm::mat4 view, glm::mat4 proj);

	// Used to fade away commands, MUST CALL!
	void update(float dt);

	void load_default_shader();

	DebugDraw();
	~DebugDraw();
};

