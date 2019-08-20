#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imnodes/imnodes.h>

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
#include "render/renderlow/drawables/dmodel.h"

#include <stb/stb_image.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;


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
	imnodes::Initialize();


	Shader test = Shader("res/shaders/test.vs", "res/shaders/test.fs");
	g_shader = &test;

	Shader debug_shader = Shader("res/shaders/debug.vs", "res/shaders/debug.fs");
	d_shader = &debug_shader;

	Shader planet_tile_shader = Shader("res/shaders/planet_tile.vs", "res/shaders/planet_tile.fs");

	//Shader atmosphere_shader = Shader("res/shaders/planet/atmosphere.vs", "res/shaders/planet/atmosphere.fs");

	//Mesh sphere_mesh = Mesh();
	//sphere_mesh.load_from_obj("res/model/icosphere.obj");
	//sphere_mesh.build_array();
	//sphere_mesh.upload();
	//DModel atmosphere = DModel(sphere_mesh, &atmosphere_shader);
	//atmosphere.tform.scl = glm::vec3(1.015f, 1.015f, 1.015f);

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

	Planet planet; planet.radius = 6378000.0f;
	planet.radius = 10.0f;
	QuadTreePlanet planet_qtree(&planet, &planet_tile_shader);
	glm::dvec2 focusPoint(0.75, 0.75);
	QuadTreeNode* onNode = &planet_qtree.px;

	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec2 mouse_look = glm::vec2(-3.14f, 3.14f / 2.0f);
	glm::vec3 position = glm::vec3(1.8f, 0.0f, -1.0f);

	int qtree_depth = 0;
	float qtree_timer = 0.0f;

	glm::dvec2 old_mouse_pos = glm::dvec2(0.0, 0.0);
	float speed = 1.0f;

	while (!glfwWindowShouldClose(window))
	{

		//v_point = glm::vec3(0.1f, 4.0f, 0.1f);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		// render
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		clock_t begin = clock();

		planet_qtree.flatten();

		auto node = onNode->get_recursive(focusPoint, qtree_depth);
		//planet_qtree.make_all_leafs_at_least(3, false);
		planet_qtree.draw_gui_window(focusPoint, onNode);

		float focusSpeed = 0.25f;
		float moveSpeed = 1.0f;
		float zoomSpeed = 0.25f;
		float speedSpeed = 0.1f;

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			focusSpeed = 0.05f;
			moveSpeed = 0.05f;
			zoomSpeed = 0.015f;
			speedSpeed = 0.01f;
		}


		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		{
			focusSpeed /= 64.0f;
			moveSpeed /= 64.0f;
			zoomSpeed /= 64.0f;
			speedSpeed /= 64.0f;
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


		if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
		{
			qtree_timer += 2.0f * dt;
			if (qtree_timer >= 1.0f)
			{
				if (planet_qtree.tile_server.is_built())
				{
					qtree_depth++;
				}
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

		glm::dvec2 new_mouse_pos;
		glfwGetCursorPos(window, &new_mouse_pos.x, &new_mouse_pos.y);


		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

			glm::dvec2 mouse_delta = new_mouse_pos - old_mouse_pos;
			mouse_look.x += (float)mouse_delta.x * dt * 0.05f;
			mouse_look.y += (float)mouse_delta.y * dt * 0.05f;
			if (mouse_look.y <= 0.001f)
			{
				mouse_look.y = 0.001f;
			}

			if (mouse_look.y >= 3.1f)
			{
				mouse_look.y = 3.1f;
			}
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		
		old_mouse_pos = new_mouse_pos;


		glm::vec3 look_at;
		look_at.x = sin(mouse_look.y) * cos(mouse_look.x);
		look_at.y = cos(mouse_look.y);
		look_at.z = sin(mouse_look.y) * sin(mouse_look.x);
		if (up != glm::vec3(0.0f, 1.0f, 0.0f))
		{
			look_at = glm::normalize(MathUtil::rotate_from_to(glm::vec3(0.0f, 1.0f, 0.0f), up) * glm::vec4(look_at, 1.0f));
		}
		glm::vec3 look_at_right = glm::normalize(glm::cross(look_at, up));

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			position += look_at * dt * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			position -= look_at_right * dt * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			position -= look_at * dt * speed;
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			position += look_at_right * dt * speed;
		}

		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			up = glm::normalize(position);
		}


		if (scroll_delta != 0)
		{
			speed += (float)scroll_delta * speedSpeed;
			if (speed <= 0.0f)
			{
				speed = 0.001f;
			}
		}

		ImGui::BeginMainMenuBar();
		ImGui::Text("Speed: %f", speed);
		ImGui::EndMainMenuBar();

		glm::vec3 norm_pos = glm::normalize(position);
		auto side = planet_qtree.get_planet_side(norm_pos);
		glm::dvec2 pos = planet_qtree.get_planet_side_position(norm_pos, side);

		focusPoint = pos;
		if (side == PlanetTilePath::PX)			{ onNode = &planet_qtree.px; }
		else if (side == PlanetTilePath::PY)	{ onNode = &planet_qtree.py; }
		else if (side == PlanetTilePath::PZ)	{ onNode = &planet_qtree.pz; }
		else if (side == PlanetTilePath::NX)	{ onNode = &planet_qtree.nx; }
		else if (side == PlanetTilePath::NY)	{ onNode = &planet_qtree.ny; }
		else if (side == PlanetTilePath::NZ)	{ onNode = &planet_qtree.nz; }

		glm::mat4 view = glm::lookAt(
			position, position + look_at, up);
		glm::mat4 proj = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.00001f, 2.0f);
		planet_qtree.draw(view, proj);

		// Draw atmosphere, we need to adjust culling
		// and enable additive blending
	
		//if (glm::length(position) <= glm::abs(atmosphere.tform.scl.x))
		//{
		//	glCullFace(GL_FRONT);
		//}
		
		//atmosphere_shader.setvec3("camPosition", position);

		//glBlendFunc(GL_ONE, GL_ONE);	//< Additive blending
		//atmosphere.draw(view, proj);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	//< Normal blending

		// glCulLFace(GL_BACK);

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