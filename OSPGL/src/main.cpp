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

#include "game/ui/orbit_view.h"

#include "game/ui/orbit_predictor.h"
#include "game/vessel/engine/rocket_engine.h"
#include "game/ui/ui_manager.h"
#include "game/ui/navball/navball.h"

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

	glm::mat4 view = glm::mat4();
	view = glm::lookAt(glm::vec3(1, 0, 1), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 proj = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.01f, 2000.0f);


	const GLubyte *ver = glGetString(GL_VERSION);
	log->info("OpenGL version: {}", ver);

	double t = 0.0f;

	glm::vec3 v_point;

	DebugDraw debug_draw;

	std::string system_data = FileUtil::load_file("res/systems/earth_moon.txt");

	SpaceSystem system;
	system.deserialize(system_data);

	NewtonBody newton;
	newton.id = "Mark I";
	newton.state.pos = glm::dvec3(384399000 / 1000.6f, 0, -384399000 / 10.1f);
	newton.state.delta = glm::dvec3(4370, 0, 300);
	//newton.state.pos = glm::dvec3(384399000 / 100.6f, 0, -384399000 / 3.0f);
	//newton.state.delta = glm::dvec3(3700, 0, 300);

	system.newton_bodies.push_back(&newton);

	OrbitView orbit_view = OrbitView(&system);

	//system.newton_bodies.push_back(&newton);

	OrbitPredictor predictor = OrbitPredictor(&system);
	predictor.def_frame.center = system.bodies[0];
	predictor.show_ui = true;

	glfwSetScrollCallback(window, &glfw_scrollwheel_callback);

	bool logged = false;

	ui_manager.timewarp = 1000.0f;

	RocketEngine engine;

	Navball navball;


	float dt = 0.0f;
	float physics_timer = 0.0f;
	double prev_time = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		v_point = glm::vec3(sin(t) * 6.8f, 3.0f, cos(t) * 6.8f);
		//v_point = glm::vec3(0.1f, 4.0f, 0.1f);
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();


		// render
		glClearColor(0.06f, 0.0f, 0.06f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		clock_t begin = clock();

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{

			system.newton_bodies[0]->state.delta += system.newton_bodies[0]->state.get_forward() * 1.0;


			predictor.thruster_clear_current += 0.01;
			if (predictor.thruster_clear_current >= predictor.thruster_clear_time)
			{
				predictor.thruster_clear_current = 0;
				predictor.mtx.lock();
				predictor.def_future.clear();
				predictor.def_future_clear = true;
				predictor.mtx.unlock();
			}

			predictor.def_predictor_sets.predictor_dt = 10.0;
		}
		else
		{
			if (predictor.def_predictor_sets.predictor_dt == 10.0)
			{
				predictor.def_predictor_sets.predictor_dt = 1.0;
				predictor.thruster_clear_current = 0;
				predictor.mtx.lock();
				predictor.def_future.clear();
				predictor.def_future_clear = true;
				predictor.mtx.unlock();
			}
		}

		physics_timer += dt;
		if (physics_timer >= 1.0f / ui_manager.timewarp)
		{
			system.simulate(ui_manager.timewarp, fixed_step, &t, NewtonBody::SolverMethod::EULER);
			physics_timer = 0.0f;
		}

	
		predictor.update(ui_manager.timewarp * fixed_step, system.time, newton.state);
		clock_t end = clock();
		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

		system.draw_debug_data(&debug_draw, 10e7);

		ui_manager.draw();

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			//system.newton_bodies[0]->state.angular_momentum -= system.newton_bodies[0]->state.get_up() * 0.000001;
			system.newton_bodies[0]->state.angular_momentum += glm::vec3(0.0f, 0.0f, 1.0f) * 0.000001f;
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			//system.newton_bodies[0]->state.angular_momentum += system.newton_bodies[0]->state.get_up() * 0.000001;
			system.newton_bodies[0]->state.angular_momentum -= glm::vec3(0.0f, 0.0f, 1.0f) * 0.000001f;
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		{
			//system.newton_bodies[0]->state.angular_momentum += system.newton_bodies[0]->state.get_right() * 0.000001;
			system.newton_bodies[0]->state.angular_momentum += glm::vec3(1.0f, 0.0f, 0.0f) * 0.000001f;
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			//system.newton_bodies[0]->state.angular_momentum -= system.newton_bodies[0]->state.get_right() * 0.000001;
			system.newton_bodies[0]->state.angular_momentum -= glm::vec3(1.0f, 0.0f, 0.0f) * 0.000001f;
		}

		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		{
			//system.newton_bodies[0]->state.angular_momentum -= system.newton_bodies[0]->state.get_forward() * 0.000001;ç
			system.newton_bodies[0]->state.angular_momentum -= glm::vec3(0.0f, 1.0f, 0.0f) * 0.000001f;
		}

		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		{
			//system.newton_bodies[0]->state.angular_momentum += system.newton_bodies[0]->state.get_forward() * 0.000001;
			system.newton_bodies[0]->state.angular_momentum += glm::vec3(0.0f, 1.0f, 0.0f) * 0.000001f;
		}
		



		glm::dvec3 pos = system.newton_bodies[0]->state.pos / 10e7;

		debug_draw.add_line(pos, pos + system.newton_bodies[0]->state.get_forward(), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
		debug_draw.add_line(pos, pos + system.newton_bodies[0]->state.get_right(), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
		debug_draw.add_line(pos, pos + system.newton_bodies[0]->state.get_up(), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		debug_draw.add_line(pos, pos + glm::dvec3(0.0, 1.0, 0.0), glm::vec4(0.0f, 0.4f, 0.0f, 1.0f));
		debug_draw.add_line(pos, pos + glm::dvec3(1.0, 0.0, 0.0), glm::vec4(0.4f, 0.0f, 0.0f, 1.0f));
		debug_draw.add_line(pos, pos + glm::dvec3(0.0, 0.0, 1.0), glm::vec4(0.0f, 0.0f, 0.4f, 1.0f));


		orbit_view.update(window, 0.005f);
		debug_draw.update(0.01f);
		debug_draw.draw(orbit_view.view, orbit_view.proj);

		orbit_view.draw();
		predictor.draw(orbit_view.view, orbit_view.proj);
		//v_point = glm::vec3(sin(t - 1.3) * 1.8f, sin((t - 1.3) / 2.0f) * 9.0f, cos(t - 1.3) * 1.8f);

		glm::quat prograde_rot = glm::conjugate(glm::toQuat(glm::lookAt(glm::vec3(0, 0, 0), (glm::vec3)glm::normalize(system.newton_bodies[0]->state.delta), 
			glm::vec3(0.0f, 1.0f, 0.0f)))) * glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		navball.draw_to_texture(system.newton_bodies[0]->state.quat_rot, prograde_rot);

		// Restore glViewport as we use a framebuffer
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

		navball.draw_to_screen(glm::ivec2(SCR_WIDTH, SCR_HEIGHT));

		engine.imgui_draw();


		// Finish
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		//orbit_view.view = glm::lookAt(v_point, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
		orbit_view.proj = proj;

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