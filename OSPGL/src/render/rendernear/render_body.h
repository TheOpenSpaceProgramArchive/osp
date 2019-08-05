#pragma once
#include "../renderlow/mesh.h"
#include "../renderspace/quad_tree_planet.h"

/*

	Handles rendering of planetary bodies (stars, planets, etc...) regardless of distance.

	When the planet is sufficiently close a Quad-tree is used to divide a sphere and generate
	LODs, at higher distances the planet switches to the far-renderer (and is scaled), and later
	on it becomes a simple billboarded dot.

	If distance is
		<= far_dist the planet is rendered as a textured sphere
		<= detail_dist the planet is rendered as a cubesphere of arbitrary detail

*/
class RenderBody
{
private:

	void generate_mesh_detailed(glm::dvec3 cameraPos, double dist);
	void generate_mesh_lowdetail(glm::dvec3 cameraPos, double dist);
	void generate_mesh_billboard(glm::dvec3 cameraPos, double dist);

	QuadTreePlanet detailed;

public:

	// Distance at which we switch to a simple billboard
	double far_dist = 1e22;
	// Distance at which we switch to high detail cubesphere
	double detail_dist = 1e10;


	Mesh mesh;

	

	// cameraPos is given relative to the planet's center
	void generate_mesh(glm::dvec3 cameraPos);

	RenderBody();
	~RenderBody();
};

