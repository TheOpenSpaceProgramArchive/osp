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

void Mesh::load_from_obj(std::string path)
{
	Logger log = spdlog::get("OSP");
	// Load the navball mesh
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str());

	if (!ret)
	{
		log->critical("Couldn't load model ({})", path);
		return;
	}

	if (!err.empty())
	{
		log->warn("Error loading obj file: {}", err);
	}

	// Check data sanity
	if (shapes.size() > 1)
	{
		log->critical("Invalid number of shapes in icosphere.obj");
		return;
	}

	size_t index_offset = 0;
	for (size_t f = 0; f < shapes[0].mesh.num_face_vertices.size(); f++)
	{
		int fv = shapes[0].mesh.num_face_vertices[f];

		if (fv > 3)
		{
			log->critical("Non-Triangulated icosphere.obj");
			return;
		}

		for (size_t v = 0; v < 3; v++)
		{
			Vertex vert;
			tinyobj::index_t idx = shapes[0].mesh.indices[index_offset + v];
			tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
			tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
			tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
			tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
			tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
			tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
			if (idx.texcoord_index > 0)
			{
				tinyobj::real_t ux = attrib.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t uy = attrib.texcoords[2 * idx.texcoord_index + 1];
				vert.uv.x = ux; vert.uv.y = uy;
			}
			

			vert.pos.x = vx; vert.pos.y = vy; vert.pos.z = vz;
			vert.nrm.x = nx; vert.nrm.y = ny; vert.nrm.z = nz;
			vert.col = glm::vec3(1.0f, 1.0f, 1.0f);
			// TODO: Read materials for color

			vertices.push_back(vert);
		}

		index_offset += fv;
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


Mesh::Mesh(const Mesh& copy)
{
	no_destroy = true;
	this->vao = copy.vao;
	this->vbo = copy.vbo;
	this->data = copy.data;
	this->vertex_count = copy.vertex_count;
	this->vertices = copy.vertices;
}

Mesh::Mesh()
{
	vao = 0;
	no_destroy = false;
}


Mesh::~Mesh()
{
	if (!no_destroy)
	{
		destroy();
	}
	
}

