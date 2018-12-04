#pragma once
#include "../../render/renderlow/drawables/dmodel.h"
#include "../../orbital/space_system.h"

// Scale between OpenGL and real units (Value is divided by this)
#define ORBIT_VIEW_SCALE (10e7)

// Minimum number of vertices to generate for an orbit, later multiplied by scale
#define ORBIT_VIEW_PRECISION 360

// Every AU of smajor axis the orbit is multiplied by this
// Done to make big orbits more precise than small ones
#define ORBIT_VIEW_PRECISION_SCALE 1.1f

// An astronomical unit in meters
#ifndef AU
#define AU 149597900000.0
#endif

// Packs a planet and its fixed keplerian orbit
// Keep in mind planetary orbits are always drawn relative to their parent
// unlike newtonian orbits whose "reference frame" can be changed.
// This could be changed in the future to allow clearer maneouvers, but for
// now it's like this to simplify the code and improve perfomance.
struct PlanetOrbitPack
{
	GLuint vbo, vao;
	size_t vert_count;
	SpaceBody* body;
};



// Handles drawing of the "map view", showing planets in small scale, alongside
// orbits and vessels, and the GUI for managing newtonian orbit viewing.
class OrbitView
{
private:

	double prev_mouse_x, prev_mouse_y;
	glm::vec3 view_pos_abs;
	float view_distance;

	float view_change_speed;

	double rot_x, rot_y;

	void update_inputs(GLFWwindow* win, float dt);


public:

	std::vector<PlanetOrbitPack> planets;

	glm::mat4 view, proj;

	void draw();
	void update(GLFWwindow* win, float dt);


	void glfw_scroll_callback(GLFWwindow* win, double xoffset, double yoffset);

	OrbitView(const SpaceSystem* system);
	~OrbitView();
};

