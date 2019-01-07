#include "navball.h"


void Navball::draw_to_texture(glm::vec3 forward)
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
	view = glm::lookAt(glm::vec3(view_distance, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.7f, 1.0f, 0.0f));
	model = glm::mat4();
	model = glm::rotate(model, ui_manager.time / 100000.0f, glm::vec3(0.5f, 0, -1));

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


	fbuffer.unbind();

	ImGui::Begin("Navball");

	ImGui::Image((void*)(intptr_t)fbuffer.tex_color_buffer, ImVec2(fbuffer.get_size().x, fbuffer.get_size().y),
		ImVec2(0, 1), ImVec2(1, 0));

	ImGui::End();
}

Navball::Navball() : 
	fbuffer(256, 256), navball_shader("res/shaders/navball/navball.vs", "res/shaders/navball/navball.fs"),
	texture("res/texture/navball/navball.png")
{
	std::shared_ptr<spdlog::logger> log = spdlog::get("OSP");

	// Load the navball mesh
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "res/model/icosphere.obj");

	if (!ret)
	{
		log->critical("Couldn't load navball model (res/model/icosphere.obj)");
		return;
	}

	if (!err.empty())
	{
		log->warn("Error loading obj file: {}", err);
	}

	// Check data sanity
	if (shapes.size() > 1)
	{
		log->critical("Invalid number of shapes in icosphere.obj");
		return;
	}

	size_t index_offset = 0;
	for (size_t f = 0; f < shapes[0].mesh.num_face_vertices.size(); f++)
	{
		int fv = shapes[0].mesh.num_face_vertices[f];

		if (fv > 3)
		{
			log->critical("Non-Triangulated icosphere.obj");
			return;
		}

		for (size_t v = 0; v < 3; v++)
		{
			tinyobj::index_t idx = shapes[0].mesh.indices[index_offset + v];
			tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
			tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
			tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
			tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
			tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
			tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

			Vertex vert;
			vert.pos.x = vx; vert.pos.y = vy; vert.pos.z = vz;
			vert.nrm.x = nx; vert.nrm.y = ny; vert.nrm.z = nz;

			navball_mesh.vertices.push_back(vert);
		}

		index_offset += fv;
	}

	navball_mesh.build_array();
	navball_mesh.upload();

	// Create framebuffers for rendering

}


Navball::~Navball()
{
}
