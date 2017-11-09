#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <iostream>
#include <rang.h>

#include "orbital/space_body.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


namespace spd = spdlog;

auto create_logger()
{
	std::vector<spdlog::sink_ptr> sinks;
	auto stdout_sink = spdlog::sinks::stdout_sink_mt::instance();
	auto color_sink = std::make_shared<spd::sinks::wincolor_stdout_sink_mt>();
	sinks.push_back(color_sink);
	// Add more sinks here, if needed.
	auto file_sink = std::make_shared<spd::sinks::simple_file_sink_mt>("output.log", "OSP_file");
	sinks.push_back(file_sink);
	auto log = std::make_shared<spd::logger>("OSP", begin(sinks), end(sinks));
	spd::register_logger(log);

	return log;
}

int main()
{
	
	auto log = create_logger();

	log->info("Initializing OSP");

	space_body sun = space_body();
	sun.parent = NULL;
	sun.mass = 1.98855 * std::pow(10, 30);

	space_body earth = space_body();
	earth.parent = &sun;
	earth.mass = 5.97219 * std::pow(10, 24);
	
	// Must be given in meters
	earth.smajor_axis = 149.6 * std::pow(10, 9);
	earth.eccentricity = 0.01671022;



	log->info("Orbital period of the earth: {}", earth.get_orbital_period());

	// Initialize GLFW

	log->info("Initializing GLFW");

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		log->critical("Could not create GLFW window, program terminating!");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	// Load callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	// Load OpenGL function pointers

	log->info("Initializing GLAD");
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		log->critical("Could not initialize GLAD, program terminating!");
		return -1;
	}

	double t = 40;

	log->info("# Time X Y");

	while (!glfwWindowShouldClose(window))
	{
		// update
		process_input(window);

		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Finish
		glfwSwapBuffers(window);
		glfwPollEvents();

		log->info("{}  {}  {}", t, earth.pos_by_time(t).x, earth.pos_by_time(t).y);

		t += 10000;
	}

	log->info("Terminating OSP");

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	spd::drop_all();

	return 0;
}


void process_input(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}