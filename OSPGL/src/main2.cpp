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
#include "render/renderspace/surface_provider.h"

#include <stb/stb_image.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

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



	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
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

	Shader debug_shader = Shader("res/shaders/debug.vs", "res/shaders/debug.fs");
	d_shader = &debug_shader;

	Shader planet_tile_shader = Shader("res/shaders/planet_tile.vs", "res/shaders/planet_tile.fs");


	// Uncomment for wireframe mode

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_CULL_FACE);

	glPointSize(4.0f);

	// Vsync
	glfwSwapInterval(1);

	glfwSetScrollCallback(window, &glfw_scrollwheel_callback);

	bool logged = false;

	ui_manager.timewarp = 1000.0f;

	float dt = 0.0f;
	float physics_timer = 0.0f;
	double prev_time = glfwGetTime();

	float t = 0.0f;

	bool wireframe = false;
	bool was_wireframe_down = false;

	Planet planet; planet.radius = 10.0; planet.surface_provider = new SurfaceProvider();
	QuadTreePlanet planet_qtree = QuadTreePlanet(&planet, &planet_tile_shader);
	glm::dvec2 focusPoint = glm::dvec2(0.75, 0.75);
	QuadTreeNode* onNode = &planet_qtree.px;

	glm::vec3 eyePoint = glm::vec3(0.0f, 3.14 / 2.0f, 2.0f);

	int qtree_depth = 0;
	float qtree_timer = 0.0f;

	while (!glfwWindowShouldClose(window))
	{

		//v_point = glm::vec3(0.1f, 4.0f, 0.1f);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		// render
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		clock_t begin = clock();

		planet_qtree.flatten();
		auto node = onNode->get_recursive(focusPoint, qtree_depth);
		planet_qtree.draw_gui_window(focusPoint, onNode);

		float focusSpeed = 0.25f;
		float moveSpeed = 1.0f;
		float zoomSpeed = 0.25f;

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			focusSpeed = 0.05f;
			moveSpeed = 0.05f;
			zoomSpeed = 0.015f;
		}


		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			focusSpeed /= 64.0f;
			moveSpeed /= 64.0f;
			zoomSpeed /= 64.0f;
		}

		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		{
			focusPoint.x -= focusSpeed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		{
			focusPoint.x += focusSpeed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		{
			focusPoint.y -= focusSpeed * dt;
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		{
			focusPoint.y += focusSpeed * dt;
		}

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			eyePoint.y -= dt * moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			eyePoint.x +=  dt * moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			eyePoint.y += dt * moveSpeed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			eyePoint.x -= dt * moveSpeed;
		}



		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		{
			eyePoint.z -= dt * zoomSpeed;
		}

		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		{
			eyePoint.z += dt * zoomSpeed;
		}

		if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		{
			qtree_timer += 2.0f * dt;
			if (qtree_timer >= 1.0f)
			{
				qtree_depth++;
				qtree_timer = 0.0f;
			}
		}

		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		{
			qtree_timer += 2.0f * dt;
			if (qtree_timer >= 1.0f)
			{
				if (qtree_depth >= 1)
				{
					qtree_depth--;
				}
				qtree_timer = 0.0f;
			}
		}

		planet_qtree.update(dt);

		glm::mat4 view = glm::lookAt(
			glm::vec3(eyePoint.z * sin(eyePoint.y) * cos(eyePoint.x), eyePoint.z * cos(eyePoint.y), eyePoint.z * sin(eyePoint.x) * sin(eyePoint.y)),
			glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 proj = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.001f, 2.5f);
		planet_qtree.draw(view, proj);


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

		t += dt;

		if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		{
			if(!was_wireframe_down)
			{
				wireframe = !wireframe;

				if (wireframe)
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				}
				else
				{
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				}
			}
			
			was_wireframe_down = true;
		}
		else
		{
			was_wireframe_down = false;
		}

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