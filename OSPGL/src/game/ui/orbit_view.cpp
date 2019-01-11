#include "orbit_view.h"

static void generate_mesh(SpaceBody* body, PlanetOrbitPack* target)
{
	std::vector<float> out;

	size_t v_count = ORBIT_VIEW_PRECISION;

	if (body->smajor_axis > AU)
	{
		v_count *= ORBIT_VIEW_PRECISION_SCALE * ((body->smajor_axis + AU) / AU);
	}

	glm::vec3 prev_pos = glm::vec3(0, 0, 0);
	for (size_t i = 0; i <= v_count; i++)
	{
		double true_an = ((double)i / (double)v_count) * 360.0;

		NewtonState state = body->to_state_origin(glm::radians(true_an));
		glm::vec3 pos = state.pos / ORBIT_VIEW_SCALE;

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

void OrbitView::glfw_scroll_callback(GLFWwindow* win, double xoffset, double yoffset)
{
	// We only care about yoffset
	view_distance += yoffset * 10.0f;
}

OrbitView::OrbitView(const SpaceSystem* system)
{
	for (size_t i = 0; i < system->bodies.size(); i++)
	{
		PlanetOrbitPack pack;
		if (system->bodies[i]->parent != NULL)
		{
			generate_mesh(system->bodies[i], &pack);
		}
		pack.body = system->bodies[i];
		pack.mesh = new DCubeSphere();
		std::string str = "res/cmaps/" + system->bodies[i]->id;
		pack.mesh->load_cubemap(str);
		pack.mesh->generate_base();

		planets.push_back(pack);
	}

	view_pos_abs = glm::vec3(1.0, 0, 0);
	view_distance = 3.0f;

	sys = system;

	vessel_indicator.load_from_obj("res/model/rocket_arrow.obj");
	vessel_indicator.build_array();
	vessel_indicator.upload();
}

void OrbitView::update_inputs(GLFWwindow* win, float dt)
{
	// Camera rotation
	double mouse_x, mouse_y;
	glfwGetCursorPos(win, &mouse_x, &mouse_y);

	if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{
		double delta_x = mouse_x - prev_mouse_x;
		double delta_y = mouse_y - prev_mouse_y;
		rot_x += (float)delta_x * dt;
		rot_y += (float)delta_y * dt;

		if (rot_y >= PI / 2.0f)
		{
			rot_y = PI / 2.0f - 0.01f;
		}

		if (rot_y <= -PI / 2.0f)
		{
			rot_y = -PI / 2.0f + 0.01f;
		}
	}
	prev_mouse_x = mouse_x;
	prev_mouse_y = mouse_y;

	view_change_speed -= scroll_delta * 9.0f;
}

void OrbitView::draw()
{
	d_shader->use();

	d_shader->setmat4("view", view);
	d_shader->setmat4("proj", proj);

	for (size_t i = 0; i < planets.size(); i++)
	{
		glm::mat4 model;

		if (planets[i].body->parent != NULL)
		{
			model = glm::translate(model, (glm::vec3)(planets[i].body->parent->last_state.pos / 10e7));
		}

		glBindVertexArray(planets[i].vao);
		glm::dvec4 col = glm::vec4(0.3, 0.3, 0.3, 1.0);
		d_shader->setvec4("color", col);
		d_shader->setmat4("model", model);

		glDrawArrays(GL_LINES, 0, planets[i].vert_count);

		glBindVertexArray(0);

		float radius_scale = (planets[i].body->radius) / 10e7;


		planets[i].mesh->model = glm::translate(glm::mat4(), (glm::vec3)(planets[i].body->last_state.pos / 10e7));
		planets[i].mesh->model = glm::scale(planets[i].mesh->model, glm::vec3(radius_scale, radius_scale, radius_scale));
		planets[i].mesh->model = glm::rotate(planets[i].mesh->model, 
			glm::radians((float)planets[i].body->last_state.rotation), planets[i].body->polar);
		planets[i].mesh->draw(view, proj);
	}

	g_shader->use();
	g_shader->setmat4("view", view);
	g_shader->setmat4("proj", proj);
	for (size_t i = 0; i < sys->newton_bodies.size(); i++)
	{
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, (glm::vec3)(sys->newton_bodies[i]->state.pos / 10e7));
		float model_scale = std::min(0.015f * std::powf(view_distance, 0.88f), 1.0f);
		model = glm::scale(model, glm::vec3(model_scale, model_scale, model_scale));

		model *= glm::toMat4(sys->newton_bodies[i]->state.quat_rot);

		g_shader->setmat4("model", model);
		glBindVertexArray(vessel_indicator.vao);
		glDrawArrays(GL_TRIANGLES, 0, vessel_indicator.vertex_count);
		glBindVertexArray(0);
	}

}

void OrbitView::update(GLFWwindow* win, float dt)
{
	update_inputs(win, dt);

	glm::vec3 focus_point = glm::vec3(sys->find_pos(focus, true) / 10e7);

	view_pos_abs = glm::vec3(cos(rot_x) * cos(rot_y), sin(rot_y), sin(rot_x) * cos(rot_y));
	view = glm::lookAt(view_pos_abs * view_distance + focus_point, focus_point, glm::vec3(0, 1, 0));

	if (view_change_speed > 0.0f)
	{
		view_change_speed -= 135.5f * dt;
	}
	else if(view_change_speed < 0.0f)
	{
		view_change_speed += 135.5f * dt;
	}


	view_distance += view_change_speed * dt;
	if (view_distance <= 0.1f)
	{
		view_distance = 0.1f;
	}

	if (std::abs(view_change_speed) <= 2.0f)
	{
		view_change_speed = 0.0f;
	}

	if (focus == "")
	{
		focus = planets[0].body->id;
	}


	// Focus window
	if (ui_manager.orbit_focus)
	{
		ImGui::Begin("View Focus", &ui_manager.orbit_focus);
		ImGui::Text("Focus: %s", focus.c_str());

		ImGui::Columns(2);
		ImGui::Text("Bodies: ");
		for (size_t i = 0; i < sys->bodies.size(); i++)
		{
			if (ImGui::Button(sys->bodies[i]->id.c_str()))
			{
				focus = sys->bodies[i]->id;
			}
		}

		ImGui::NextColumn();
		ImGui::Text("Vessels: ");

		for (size_t i = 0; i < sys->newton_bodies.size(); i++)
		{
			if (ImGui::Button(sys->newton_bodies[i]->id.c_str()))
			{
				focus = sys->newton_bodies[i]->id;
			}
		}

		ImGui::EndColumns();

		ImGui::End();
	}

}

OrbitView::~OrbitView()
{
}
