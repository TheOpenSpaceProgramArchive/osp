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

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "render/renderspace/quad_tree_planet.h"
#include "render/renderspace/planet_tile_server.h"
#include "render/renderlow/shader.h"

#include <stb/stb_image.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow *window, SpaceBody* earth);

// settings
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 800;


namespace spd = spdlog;

Shader* g_shader = NULL;
Shader* d_shader = NULL;
UIManager ui_manager;

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

double scroll_delta = 0.0;
bool scroll_delta_set_now = false;

const double fixed_step = 1.0f;

void glfw_scrollwheel_callback(GLFWwindow* win, double xoffset, double yoffset)
{
	scroll_delta = yoffset;
	scroll_delta_set_now = true;
}


int main()
{

	Logger log = create_logger();

	log->info("Initializing OSP");

	log->info("Initializing GLFW");

	QuadTreePlanet planet = QuadTreePlanet();
	glm::dvec2 focusPoint = glm::dvec2(0.75, 0.75);
	QuadTreeNode* onNode = &planet.px;



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

	GLFWimage* img;
	img = new GLFWimage();

	int ch;
	unsigned char* data = stbi_load("res/icon.png", &img->width, &img->height, &ch, 4);
	img->pixels = data;

	glfwSetWindowIcon(window, 1, img);

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

	glfwSetScrollCallback(window, &glfw_scrollwheel_callback);

	bool logged = false;

	ui_manager.timewarp = 1000.0f;

	float dt = 0.0f;
	float physics_timer = 0.0f;
	double prev_time = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{

		//v_point = glm::vec3(0.1f, 4.0f, 0.1f);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		// render
		glClearColor(0.06f, 0.0f, 0.06f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		clock_t begin = clock();

		planet.flatten();
		auto node = onNode->get_recursive(focusPoint, 8);
		planet.draw_gui_window(focusPoint, onNode);

		auto allLeafs = planet.px.getAllLeafNodes();
		auto path = node->getPath();
		PlanetTilePath ppath = PlanetTilePath();
		ppath.path = path;


		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			focusPoint.x -= 0.25f * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			focusPoint.x += 0.25f * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			focusPoint.y -= 0.25f * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			focusPoint.y += 0.25f * dt;
		}

		// Finish
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		if (scroll_delta_set_now)
		{
			scroll_delta = 0;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();

		double actual_time = glfwGetTime();
		dt = (float)(actual_time - prev_time);
		prev_time = actual_time;

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