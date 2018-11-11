#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <spdlog/spdlog.h>

#include <iostream>
#include <rang.h>
#include <chrono>

#include "orbital/space_body.h"

#include "util/defines.h"

#include "render/renderlow/shader.h"
#include "render/renderlow/mesh.h"
#include "render/renderlow/transform.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render/renderlow/drawables/dbillboard.h"
#include "render/renderlow/drawables/dcubesphere.h"
#include "render/renderlow/debug_draw.h"
#include "orbital/newton_body.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow *window, SpaceBody* earth);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;


namespace spd = spdlog;

Shader* g_shader = NULL;
Shader* d_shader = NULL;


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
	
	Logger log = create_logger();

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

	// Load OpenGL function pointers

	log->info("Initializing GLAD");
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		log->critical("Could not initialize GLAD, program terminating!");
		return -1;
	}

	// Load callbacks
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	// Setup style
	ImGui::StyleColorsDark();


	Shader test = Shader("res/shaders/test.vs", "res/shaders/test.fs");
	g_shader = &test;

	Shader debugShader = Shader("res/shaders/debug.vs", "res/shaders/debug.fs");
	d_shader = &debugShader;

	// Uncomment for wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPointSize(4.0f);

	// Vsync
	glfwSwapInterval(1);

	glm::mat4 view = glm::mat4();
	view = glm::lookAt(glm::vec3(1, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);


	const GLubyte *ver = glGetString(GL_VERSION);
	log->info("OpenGL version: {}", ver);

	float t = 0.0f;

	glm::vec3 v_point;

	DebugDraw debug_draw;

	SpaceSystem system;
	SpaceBody earth;
	earth.mass = 5.97 * 10e24;
	SpaceBody moon;
	moon.mass = 7.32 * 10e22;
	moon.smajor_axis = 384399000;
	moon.eccentricity = 0.0549;
	moon.arg_periapsis = 114;
	moon.asc_node = -11;
	moon.inclination = 5.012;
	moon.parent = &earth;

	system.bodies.push_back(&earth);
	system.bodies.push_back(&moon);

	NewtonBody newton;
	newton.state.pos = glm::dvec3(384399000 / 1.1f, 0, 0);
	newton.state.delta = glm::dvec3(0, 0, 1400);


	while (!glfwWindowShouldClose(window))
	{
		//v_point = glm::vec3(sin(t) * 6.8f, 3.0f, cos(t) * 6.8f);
		v_point = glm::vec3(0.1f, 4.0f, 0.1f);
		//ImGui_ImplOpenGL3_NewFrame();
		//ImGui_ImplGlfw_NewFrame();
		//ImGui::NewFrame();

		moon.true_anomaly = t;
		// render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		newton.state = newton.solve(system, NewtonBody::SolverMethod::EULER, 8, 1000);

		log->info("Newton State: raw({},{},{})", newton.state.delta.x, newton.state.delta.y, newton.state.delta.z);

		debug_draw.add_cross(moon.to_state().pos / 10e7, 0.05f, glm::vec4(0.4f, 0.4f, 1.0f, 1.0f));
		debug_draw.add_cross(glm::vec3(0.0f, 0.0f, 0.0f), 0.1f, glm::vec4(1.0f, 0.6f, 0.6f, 1.0f));
		debug_draw.add_cross(newton.state.pos / 10e7, 0.03f, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		debug_draw.add_point(newton.state.pos / 10e7, glm::vec4(0.7f, 0.2f, 0.2f, 1.0f), 2.0f, 4.0f);

		debug_draw.update(0.01f);
		debug_draw.draw(view, proj);

		//v_point = glm::vec3(sin(t - 1.3) * 1.8f, sin((t - 1.3) / 2.0f) * 9.0f, cos(t - 1.3) * 1.8f);

		// Finish
		//ImGui::Render();
		//ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		view = glm::lookAt(v_point, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		t += 0.01;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	log->info("Terminating OSP");

	// glfw: terminate, clearing all previously allocated GLFW resources.
	glfwTerminate();
	spd::drop_all();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}