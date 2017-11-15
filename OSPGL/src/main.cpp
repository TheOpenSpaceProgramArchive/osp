#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <iostream>
#include <rang.h>

#include "orbital/space_body.h"

#include "util/defines.h"

#include "render/renderlow/shader.h"
#include "render/renderlow/mesh.h"
#include "render/renderlow/transform.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow *window, space_body* earth);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;


namespace spd = spdlog;

auto create_logger()
{
	std::vector<spdlog::sink_ptr>* sinks = new std::vector<spdlog::sink_ptr>();
	auto stdout_sink = spdlog::sinks::stdout_sink_mt::instance();
	auto color_sink = std::make_shared<spd::sinks::wincolor_stdout_sink_mt>();
	sinks->push_back(color_sink);
	// Add more sinks here, if needed.
	auto file_sink = std::make_shared<spd::sinks::simple_file_sink_mt>("res/output.log", "OSP_file");
	sinks->push_back(file_sink);
	auto log = std::make_shared<spd::logger>("OSP", begin(*sinks), end(*sinks));
	spd::register_logger(log);

	return log;
}



int main()
{
	
	logger log = create_logger();

	log->info("Initializing OSP");

	log->info("Initializing GLFW");

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Open Space Program", NULL, NULL);
	if (window == NULL)
	{
		log->critical("Could not create GLFW window, program terminating!");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	// Load callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	space_body sun = space_body();
	sun.mass = 1.9891 * std::pow(10, 30);

	space_body earth = space_body();
	earth.parent = &sun;
	earth.smajor_axis = 1.496e+11;
	earth.eccentricity = 0;
	earth.mass = 5.972 * std::pow(10, 24);

	
	// Load OpenGL function pointers

	log->info("Initializing GLAD");
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		log->critical("Could not initialize GLAD, program terminating!");
		return -1;
	}


	shader test = shader("res/shaders/test.vs", "res/shaders/test.fs");

	double t = 40;

	mesh triangle = mesh();
	mesh lines = mesh();
	
	vertex prev;
	prev.pos.x = -123456789;

	transform lform;
	lform.build_matrix();
	
	transform tform;
	tform.build_matrix();

	triangle.vertices.push_back({ glm::vec3(0, 0, 0) }); // top
	triangle.vertices.push_back({ glm::vec3(0.05, 0.1, 0) }); // bright
	triangle.vertices.push_back({ glm::vec3(-0.05, 0.1, 0) }); // bleft

	triangle.build_array();
	triangle.upload();


	log->info("# Time X Y");

	// Uncomment for wireframe mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Vsync
	glfwSwapInterval(1);

	while (!glfwWindowShouldClose(window))
	{
		// update
		process_input(window, &earth);


		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		tform.pos = earth.pos_by_time(t);
		tform.pos /= 1.496e+11;
		tform.pos /= 4;

		lines.vertices.clear();
		// Generate vertices from orbit
		for (double a = 0; a < 2 * 3.1415; a += 0.05)
		{
			vertex vert;
			vert.pos = earth.pos_by_mean(a);

			// Transform into screen space
			vert.pos /= 1.496e+11;
			vert.pos /= 4;


			if (prev.pos.x == -123456789)
			{
				prev.pos = vert.pos;
			}


			lines.vertices.push_back(prev);

			lines.vertices.push_back(vert);

			prev = vert;
		}

		lines.build_array();
		lines.upload();

		log->info("Earth X {} Y {} Z {} (R {}) E {}", tform.pos.x, tform.pos.y, tform.pos.z, earth.get_altitude(t), earth.eccentricity);

		glUseProgram(test.program);
		test.setmat4("transform", tform.build_matrix());
		glBindVertexArray(triangle.vao);
		glDrawArrays(GL_TRIANGLES, 0, triangle.vertices.size());

		test.setmat4("transform", lform.build_matrix());
		glBindVertexArray(lines.vao);
		glDrawArrays(GL_LINES, 0, lines.vertices.size());

		// Finish
		glfwSwapBuffers(window);
		glfwPollEvents();

		t += 100000;
	}

	log->info("Terminating OSP");

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	spd::drop_all();

	return 0;
}


void process_input(GLFWwindow *window, space_body* earth)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		earth->eccentricity -= 0.004;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		earth->eccentricity += 0.004;
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}