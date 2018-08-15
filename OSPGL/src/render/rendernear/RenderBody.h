#pragma once
#include "../renderlow/mesh.h"


/*

	Handles rendering of planetary bodies (stars, planets, etc...) regardless of distance.

	When the planet is sufficiently close a Quad-tree is used to divide a sphere and generate
	LODs, at higher distances the planet switches to the far-renderer (and is scaled), and later
	on it becomes a simple billboarded dot.

*/
class RenderBody
{
public:

	// Distance at which we switch to low-detailed scaled-space
	double detail_dist = 1e10;
	// Distance at which we switch to a simple billboard
	double far_dist = 1e22;

	Mesh mesh;

	void generate_mesh(double dist);

	RenderBody();
	~RenderBody();
};

