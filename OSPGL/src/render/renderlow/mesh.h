#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "../../util/defines.h"

struct vertex
{
	glm::vec3 pos;
};


/*
	Meshes are arrays of triangles with associated vertex
	positions, normals, texture coordinates and tangents.

	They are stored and used by mesh_renderers to draw
	different shapes.

	By themselves they are nothing more than a way to store
	information and manage it.

*/


class mesh
{
private:


	// Data used internally
	std::vector<float> data;

public:

	uint vao = 0, vbo = 0;


	std::vector<vertex> vertices;

	// Builds the low-level GL compatible array
	void build_array();
	
	// Uploads the data to the GPU, making VAO and VBO valid
	void upload();

	void clear_buffers();

	mesh();
	~mesh();
};

