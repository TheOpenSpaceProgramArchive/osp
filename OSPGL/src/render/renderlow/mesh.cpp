#include "mesh.h"



void mesh::build_array()
{
	// For now simply place the positions
	data.clear();
	data.resize(vertices.size() * 6);

	size_t j = 0;

	for (size_t i = 0; i < vertices.size(); i++)
	{
		data[j + 0] = vertices[i].pos.x;
		data[j + 1] = vertices[i].pos.y;
		data[j + 2] = vertices[i].pos.z;

		data[j + 3] = vertices[i].col.r;
		data[j + 4] = vertices[i].col.g;
		data[j + 5] = vertices[i].col.b;

		j += 6;
	}
}

void mesh::upload()
{
	clear_buffers();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	{
		// Generate VBO (Vertex buffer object)
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		{
			glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(data), data.data(), GL_STATIC_DRAW);

			// position
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			// color
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);

}

void mesh::clear_buffers()
{
	if (vao != 0)
	{
		glDeleteBuffers(1, &vao);
	}
	if (vbo != 0)
	{
		glDeleteBuffers(1, &vbo);
	}
}


mesh::mesh()
{
}


mesh::~mesh()
{
}
