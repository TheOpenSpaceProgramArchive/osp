#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "../../util/defines.h"


struct Vertex
{
	glm::vec3 pos;
	glm::vec3 col;
	glm::vec3 nrm;
	glm::vec2 uv;
};


/*
	Meshes are arrays of triangles with associated vertex
	positions, normals, texture coordinates and tangents.

	They are stored and used by mesh_renderers to draw
	different shapes.

	By themselves they are nothing more than a way to store
	information and manage it.

*/


class Mesh
{
private:


	// Data used internally
	std::vector<float> data;

public:

	uint vao = 0, vbo = 0;


	std::vector<Vertex> vertices;

	// Builds the low-level GL compatible array
	void build_array();
	
	// Uploads the data to the GPU, making VAO and VBO valid
	void upload();

	void gen_buffers();

	// Assumes triangles in order
	void generate_normals(bool smooth = true, bool flip = false);

	Mesh();
	~Mesh();
};

struct GlmVec3Compare
{

	bool operator()(const glm::vec3& lhs, const glm::vec3& rhs) const
	{
		return lhs.x < rhs.x
			|| (lhs.x == rhs.x && (lhs.y < rhs.y
				|| (lhs.y == rhs.y && lhs.z < rhs.z)));
	}
};