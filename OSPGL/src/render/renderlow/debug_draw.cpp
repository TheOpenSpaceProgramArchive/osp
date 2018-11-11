#include "debug_draw.h"


void DebugDrawCommand::generate(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
{
	int vertCount;
	if (draw_mode == GL_POINTS)
	{
		vertCount = 1;
	}
	else if (draw_mode = GL_LINES)
	{
		vertCount = 2;
	}
	else
	{
		vertCount = 3;
	}

	float points[9];
	points[0] = p0.x; points[1] = p0.y; points[2] = p0.z;
	points[3] = p1.x; points[4] = p1.y; points[5] = p1.z;
	points[6] = p2.x; points[7] = p2.y; points[8] = p2.z;

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertCount * 3 * sizeof(float), points, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

}


void DebugDraw::add_point(glm::vec3 pos, glm::vec4 color, float point_size, float time)
{
	DebugDrawCommand cmd;
	cmd.time = time;
	cmd.draw_mode = GL_POINTS;
	cmd.color = color;
	cmd.point_size = point_size;
	cmd.generate(pos, pos, pos);
	commands.push_back(cmd);
}

void DebugDraw::add_line(glm::vec3 pos0, glm::vec3 pos1, glm::vec4 color, float time)
{
	DebugDrawCommand cmd;
	cmd.time = time;
	cmd.draw_mode = GL_LINES;
	cmd.color = color;
	cmd.generate(pos0, pos1, pos1);
	commands.push_back(cmd);
}

void DebugDraw::add_cross(glm::vec3 pos, float radius, glm::vec4 color, float time)
{
	DebugDrawCommand line0, line1, line2;
	line0.time = time; line1.time = time; line2.time = time;
	line0.color = color; line1.color = color; line2.color = color;
	line0.draw_mode = GL_LINES; line1.draw_mode = GL_LINES; line2.draw_mode = GL_LINES;

	glm::vec3 line0point0 = pos + glm::vec3(-radius, 0, 0);
	glm::vec3 line0point1 = pos + glm::vec3(radius, 0, 0);
	glm::vec3 line1point0 = pos + glm::vec3(0, -radius, 0);
	glm::vec3 line1point1 = pos + glm::vec3(0, radius, 0);
	glm::vec3 line2point0 = pos + glm::vec3(0, 0, -radius);
	glm::vec3 line2point1 = pos + glm::vec3(0, 0, radius);

	line0.generate(line0point0, line0point1, line0point1);
	line1.generate(line1point0, line1point1, line1point1);
	line2.generate(line2point0, line2point1, line2point1);

	commands.push_back(line0);
	commands.push_back(line1);
	commands.push_back(line2);
}

void DebugDraw::draw(glm::mat4 view, glm::mat4 proj)
{
	if (shader == NULL)
	{
		load_default_shader();
	}


	shader->use();
	shader->setmat4("view", view);
	shader->setmat4("proj", proj);

	for (auto it = commands.begin(); it != commands.end();)
	{
		int vertCount;
		if (it->draw_mode == GL_POINTS)
		{
			vertCount = 1;
		}
		else if (it->draw_mode = GL_LINES)
		{
			vertCount = 2;
		}
		else
		{
			vertCount = 3;
		}

		glPointSize(it->point_size);

		glBindVertexArray(it->vao);
		if (it->time <= 1.0f && it->time != 0.0f)
		{
			glm::dvec4 col = it->color;
			col.a *= it->time;
			shader->setvec4("color", col);
		}
		else
		{
			shader->setvec4("color", it->color);
		}
		
		glDrawArrays(it->draw_mode, 0, vertCount);

		if (it->time <= 0.0f)
		{
			glDeleteVertexArrays(1, &it->vao);
			glDeleteBuffers(1, &it->vbo);
			it = commands.erase(it);
		}
		else
		{
			it++;
		}
	}
}

void DebugDraw::update(float dt)
{
	for (size_t i = 0; i < commands.size(); i++)
	{
		if (commands[i].time != 0.0f)
		{
			commands[i].time -= dt;
		}
	}
}

void DebugDraw::load_default_shader()
{
	this->shader = d_shader;
}

DebugDraw::DebugDraw()
{
}


DebugDraw::~DebugDraw()
{
}

