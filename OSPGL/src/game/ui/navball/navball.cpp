#include "navball.h"


void Navball::draw_to_texture(glm::quat rot)
{
	// Calculate the FOV from view distance
	if (view_distance <= 1.0f)
	{
		view_distance = 1.0f;
	}

	// Simple trigonometry leads to this
	// Given the tangent point Q, the view point P and the 
	// circle center C, the triangle PQC is rectangle (as Q is the tangent point)
	// we know the sine as we know the opposite to the angle (1.0f) and the
	// hypothenuse (view_distance)
	float angle_sin = 1.0f / view_distance;
	float fov = std::asinf(angle_sin) * 2.0f;

	glm::mat4 view, proj, model;
	// 60 degrees as we see the sphere from twice its radius
	// (Hence we form an equilateral triangle)
	proj = glm::perspective(fov, 1.0f, 0.01f, 255.0f);
	view = glm::lookAt(glm::vec3(0.0f, view_distance, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.7f, 1.0f, 0.0f));
	model = glm::mat4();
	model = glm::scale(model, glm::vec3(scale, scale, scale));
	model *= glm::toMat4(glm::conjugate(rot));

	fbuffer.bind();
	fbuffer.set_viewport();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	navball_shader.use();


	navball_shader.setmat4("view", view);
	navball_shader.setmat4("proj", proj);
	navball_shader.setmat4("model", model);
	navball_shader.seti("tex", 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.texture);

	glBindVertexArray(navball_mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, navball_mesh.vertex_count);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	frame_sprite.draw(glm::mat4(), glm::mat4());

	fbuffer.unbind();

	/*ImGui::Begin("Navball");

	ImGui::Image((void*)(intptr_t)fbuffer.tex_color_buffer, ImVec2(fbuffer.get_size().x, fbuffer.get_size().y),
		ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();*/
}

void Navball::draw_to_screen(glm::ivec2 screen_size)
{
	float x_size = fbuffer.get_size().x * final_scale;
	float y_size = fbuffer.get_size().x * final_scale;

	glm::mat4 view = glm::mat4();
	glm::mat4 proj = glm::ortho(0.0f, (float)screen_size.x, 0.0f, (float)screen_size.y, 0.01f, 100.0f);
	final_sprite.tform.pos = glm::vec3(screen_size.x / 2.0f, y_size, -1.0f);
	final_sprite.tform.scl = glm::vec3(y_size, -y_size, 1.0f);
	glDisable(GL_DEPTH_TEST);
	final_sprite.draw(view, proj);
	glEnable(GL_DEPTH_TEST);
}

Navball::Navball() :
	fbuffer(256, 256), navball_shader("res/shaders/navball/navball.vs", "res/shaders/navball/navball.fs"),
	texture("res/texture/navball/navball.png"), frame_sprite("res/texture/navball/frame.png"), final_sprite(0)
{
	std::shared_ptr<spdlog::logger> log = spdlog::get("OSP");

	navball_mesh.load_from_obj("res/model/icosphere.obj");
	navball_mesh.build_array();
	navball_mesh.upload();

	final_sprite.texture = fbuffer.tex_color_buffer;

}


Navball::~Navball()
{
}
