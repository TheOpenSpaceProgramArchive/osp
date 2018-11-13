#include "orbit_view.h"

static void generate_mesh(SpaceBody* body, PlanetOrbitPack* target)
{
	std::vector<float> out;

	double or_true = body->true_anomaly;

	size_t v_count = ORBIT_VIEW_PRECISION;

	if (body->smajor_axis > AU)
	{
		v_count *= ORBIT_VIEW_PRECISION_SCALE * ((body->smajor_axis + AU) / AU);
	}

	glm::vec3 prev_pos = glm::vec3(0, 0, 0);
	for (size_t i = 0; i <= v_count; i++)
	{
		double true_an = ((double)i / (double)v_count) * 360.0;
		
		body->true_anomaly = glm::radians(true_an);

		NewtonState state = body->to_state();
		glm::vec3 pos = state.pos / 10e7;

		if (prev_pos.x == 0 && prev_pos.y == 0 && prev_pos.z == 0)
		{
			prev_pos = pos;
		}
		out.push_back(prev_pos.x); out.push_back(prev_pos.y); out.push_back(prev_pos.z);
		out.push_back(pos.x); out.push_back(pos.y); out.push_back(pos.z);

		prev_pos = pos;

	}

	glGenVertexArrays(1, &target->vao);
	glGenBuffers(1, &target->vbo);

	glBindVertexArray(target->vao);
	glBindBuffer(GL_ARRAY_BUFFER, target->vbo);
	glBufferData(GL_ARRAY_BUFFER, v_count * 3 * sizeof(float) * 2 + (2 * 3 * sizeof(float)), out.data(), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	target->vert_count = (v_count * 2) + 2;
}

OrbitView::OrbitView(const SpaceSystem* system)
{
	for (size_t i = 0; i < system->bodies.size(); i++)
	{
		if (system->bodies[i]->parent != NULL)
		{
			PlanetOrbitPack pack;
			generate_mesh(system->bodies[i], &pack);
			
			planets.push_back(pack);
		}
	}
}

void OrbitView::draw(glm::mat4 view, glm::mat4 proj)
{
	d_shader->use();
	d_shader->setmat4("view", view);
	d_shader->setmat4("proj", proj);

	for (size_t i = 0; i < planets.size(); i++)
	{
		glBindVertexArray(planets[i].vao);
		glm::dvec4 col = glm::vec4(0.3, 0.3, 0.3, 1.0);
		d_shader->setvec4("color", col);

		glDrawArrays(GL_LINES, 0, planets[i].vert_count);

		glBindVertexArray(0);
	}
}

OrbitView::~OrbitView()
{
}
