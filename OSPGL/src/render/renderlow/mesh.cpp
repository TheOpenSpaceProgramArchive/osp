#include "mesh.h"

void Mesh::destroy()
{
	vertices.clear();

	if (vao != 0)
	{
		glDeleteVertexArrays(1, &vao);
		vao = 0;
	}

	if (vbo != 0)
	{
		glDeleteBuffers(1, &vbo);
		vbo = 0;
	}
}

void Mesh::build_array()
{
	// For now simply place the positions
	vertex_count = vertices.size();
	data.clear();
	data.resize(vertices.size() * 11);

	size_t j = 0;

	for (size_t i = 0; i < vertices.size(); i++)
	{
		data[j + 0] = vertices[i].pos.x;
		data[j + 1] = vertices[i].pos.y;
		data[j + 2] = vertices[i].pos.z;

		data[j + 3] = vertices[i].col.r;
		data[j + 4] = vertices[i].col.g;
		data[j + 5] = vertices[i].col.b;

		data[j + 6] = vertices[i].nrm.r;
		data[j + 7] = vertices[i].nrm.g;
		data[j + 8] = vertices[i].nrm.b;

		data[j + 9] = vertices[i].uv.x;
		data[j + 10] = vertices[i].uv.y;

		j += 11;
	}
}

void Mesh::generate_normals(bool smooth, bool flip)
{
	if (smooth)
	{
		std::map<glm::vec3, glm::vec3, GlmVec3Compare> unique_verts;

		for (size_t i = 0; i < vertices.size(); i+=3)
		{
			Vertex* a = &vertices[i];
			Vertex* b = &vertices[i + 1];
			Vertex* c = &vertices[i + 2];

			glm::vec3 n = glm::cross(b->pos - a->pos, c->pos - a->pos);

			if (unique_verts.count(a->pos) == 0)
			{
				unique_verts[a->pos] = glm::vec3(0, 0, 0);
			}
			if (unique_verts.count(b->pos) == 0)
			{
				unique_verts[b->pos] = glm::vec3(0, 0, 0);
			}
			if (unique_verts.count(c->pos) == 0)
			{
				unique_verts[c->pos] = glm::vec3(0, 0, 0);
			}

			unique_verts[a->pos] += n;
			unique_verts[b->pos] += n;
			unique_verts[c->pos] += n;
		}

		for (size_t i = 0; i < vertices.size(); i++)
		{
			vertices[i].nrm = glm::normalize(unique_verts[vertices[i].pos]);
			if (flip)
			{
				vertices[i].nrm = -vertices[i].nrm;
			}
		}

	}
	else
	{
		for (size_t i = 0; i < vertices.size(); i += 3)
		{
			Vertex* a = &vertices[i];
			Vertex* b = &vertices[i + 1];
			Vertex* c = &vertices[i + 2];

			glm::vec3 n = glm::normalize(glm::cross(b->pos - a->pos, c->pos - a->pos));
			if(flip)
			{
				n = -n;
			}

			a->nrm = n; b->nrm = n; c->nrm = n;
		}
	}
}

void Mesh::upload(bool dynamic)
{
	bool existed = gen_buffers();

	glBindVertexArray(vao);
	{
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		{
			if (dynamic)
			{
				glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(data[0]), data.data(), GL_DYNAMIC_DRAW);
			}
			else
			{
				glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(data[0]), data.data(), GL_STATIC_DRAW);
			}
			

			if (!existed)
			{
				// position
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				// color
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);
				// normal
				glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
				glEnableVertexAttribArray(2);
				// tex
				glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(9 * sizeof(float)));
				glEnableVertexAttribArray(3);
			}
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);

}

bool Mesh::gen_buffers()
{
	bool ret = false;

	if (vao == 0)
	{
		glGenVertexArrays(1, &vao);
	}
	else
	{
		ret = true;
	}

	if (vbo == 0)
	{
		glGenBuffers(1, &vbo);
	}
	else
	{
		ret = true;
	}

	return ret;
}


Mesh::Mesh()
{
}


Mesh::~Mesh()
{
	destroy();
}

