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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render/renderlow/drawables/dbillboard.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow *window, SpaceBody* earth);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;


namespace spd = spdlog;

Shader* g_shader = NULL;

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


	SpaceBody earth = SpaceBody();
	earth.mass = 5.972 * std::pow(10, 24);

	SpaceBody vessel = SpaceBody();
	vessel.parent = &earth;
	vessel.smajor_axis = 6775 * 1000;
	vessel.eccentricity = 0.0023;
	vessel.inclination = 0.004;
	vessel.mass = 100;

	
	// Load OpenGL function pointers

	log->info("Initializing GLAD");
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		log->critical("Could not initialize GLAD, program terminating!");
		return -1;
	}


	Shader test = Shader("res/shaders/test.vs", "res/shaders/test.fs");
	g_shader = &test;

	Shader planet = Shader("res/shaders/planet.vs", "res/shaders/planet.fs");

	double t = 40;

	Mesh triangle = Mesh();
	Mesh lines = Mesh();
	Mesh vector = Mesh();
	
	Vertex prev;
	prev.pos.x = -123456789;

	Transform lform;
	lform.build_matrix();
	
	Transform tform;
	tform.build_matrix();

	triangle.vertices.push_back({ glm::vec3(0, 0, 0) }); // top
	triangle.vertices.push_back({ glm::vec3(0.05, 0.1, 0) }); // bright
	triangle.vertices.push_back({ glm::vec3(-0.05, 0.1, 0) }); // bleft

	triangle.build_array();
	triangle.upload();


	log->info("# Time X Y");

	// Uncomment for wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Vsync
	glfwSwapInterval(1);

	DBillboard bboard = DBillboard();
	bboard.Shader = &planet;
	bboard.tform.pos = { 0, 0, 0 };
	bboard.tform.scl = { 2.6, 2.6, 2.6 };

	log->info("Orbital Period {}s", vessel.get_orbital_period());

	while (!glfwWindowShouldClose(window))
	{
		// update
		process_input(window, &vessel);


		// render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		tform.pos = vessel.pos_by_time(t);
		tform.pos /= 200 * 1000;
		tform.pos /= 4;
		tform.rot = glm::quat(0, 0, 1, 1);

		lines.vertices.clear();

		glm::mat4 view;

		//view = glm::translate(glm::vec3(sin(t / 10000000), 0, cos(t / 10000000)));

		view = glm::lookAt(glm::vec3(0.001f, 3, 4) , glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		glm::mat4 proj;
		proj = glm::perspective(glm::radians(55.0f), (float)(SCR_WIDTH / SCR_HEIGHT), 0.05f, 100.0f);


		test.setmat4("proj", proj);
		test.setmat4("view", view);


		// Generate vertices from orbit
		for (double a = 0; a < 2 * PI; a += 0.05)
		{
			Vertex vert;
			vert.pos = vessel.pos_by_mean(a);

			// Transform into screen space
			vert.pos /= 6000 * 1000;

			if (a <= 0.1)
			{
				vert.col = { 1.0, 0.0, 1.0 };
			}
			else if (a <= PI + 0.1 && a >= PI - 0.1)
			{
				vert.col = { 1.0, 1.0, 0.0 };
			}
			else if (a <= glm::radians(vessel.asc_node) + 0.1 && a >= glm::radians(vessel.asc_node) - 0.1)
			{
				vert.col = { 1.0, 1.0, 1.0 };
			}
			else
			{
				//vert.col.r = a / (2 * 3.1415);
				vert.col.g = vessel.get_altitude_mean(a) / (2 * vessel.smajor_axis);
			}

			if (prev.pos.x == -123456789)
			{
				prev.pos = vert.pos;
				prev.col = vert.col;
			}


			lines.vertices.push_back(prev);

			lines.vertices.push_back(vert);

			prev = vert;
		}

		lines.build_array();
		lines.upload();


		NewtonState st = vessel.state_by_time(t);

		// Build vec
		vector.vertices.clear();
		vector.vertices.push_back({ tform.pos });
		vector.vertices.push_back({ tform.pos + ((glm::vec3)(st.delta / (double)30000)) });
		vector.build_array();
		vector.upload();

		//log->info("Earth AP: {} PE: {} E: {} I {} AN: {}", vessel.get_apoapsis_radius(), vessel.get_periapsis_radius(),
		//vessel.eccentricity, vessel.inclination, vessel.asc_node);
		
		glUseProgram(test.program);
		test.setmat4("model", tform.build_matrix());
		glBindVertexArray(triangle.vao);
		glDrawArrays(GL_TRIANGLES, 0, triangle.vertices.size());

		test.setmat4("model", lform.build_matrix());
		glBindVertexArray(vector.vao);
		glDrawArrays(GL_LINES, 0, vector.vertices.size());

		test.setmat4("model", lform.build_matrix());
		glBindVertexArray(lines.vao);
		glDrawArrays(GL_LINES, 0, lines.vertices.size());
		
		bboard.draw(view, proj);

		// Finish
		glfwSwapBuffers(window);
		glfwPollEvents();

		t += 1;
	}

	log->info("Terminating OSP");

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	spd::drop_all();

	return 0;
}


void process_input(GLFWwindow *window, SpaceBody* earth)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		earth->inclination -= 0.4;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		earth->inclination += 0.4;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		earth->asc_node -= 0.4;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		earth->asc_node += 0.4;
	}

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
	{
		earth->eccentricity -= 0.004;
	}

	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
	{
		earth->eccentricity += 0.004;
	}

	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
	{
		earth->arg_periapsis -= 0.4;
	}

	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
	{
		earth->arg_periapsis += 0.4;
	}

	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
	{
		NewtonState st = earth->state_by_mean(50);
		earth->set_state(st);
	}

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}