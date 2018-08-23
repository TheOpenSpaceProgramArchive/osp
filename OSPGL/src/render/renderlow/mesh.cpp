#include "mesh.h"



void Mesh::build_array()
{
	// For now simply place the positions
	data.clear();
	data.resize(vertices.size() * 8);

	size_t j = 0;

	for (size_t i = 0; i < vertices.size(); i++)
	{
		data[j + 0] = vertices[i].pos.x;
		data[j + 1] = vertices[i].pos.y;
		data[j + 2] = vertices[i].pos.z;

		data[j + 3] = vertices[i].col.r;
		data[j + 4] = vertices[i].col.g;
		data[j + 5] = vertices[i].col.b;

		data[j + 6] = vertices[i].uv.x;
		data[j + 7] = vertices[i].uv.y;

		j += 8;
	}
}

void Mesh::upload()
{
	gen_buffers();

	glBindVertexArray(vao);
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		{
			glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(data[0]), data.data(), GL_STATIC_DRAW);

			// position
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			// color
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			// tex
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glEnableVertexAttribArray(2);
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);

}

void Mesh::gen_buffers()
{
	if (vao == 0)
	{
		glGenVertexArrays(1, &vao);
	}
	if (vbo == 0)
	{
		glGenBuffers(1, &vbo);
	}
}


Mesh::Mesh()
{
}


Mesh::~Mesh()
{
	if (vao != 0)
	{
		glDeleteVertexArrays(1, &vao);
	}

	if (vbo != 0)
	{
		glDeleteBuffers(1, &vbo);
	}
}

