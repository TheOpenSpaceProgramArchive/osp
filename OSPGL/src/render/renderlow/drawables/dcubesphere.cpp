#include "dcubesphere.h"


std::vector<glm::vec3> DCubeSphere::make_cube_face(size_t detail, glm::mat4 tform)
{
	std::vector<glm::vec3> out;

	float step = 1.0 / (float)detail;

	for (size_t x = 0; x <= detail; x++)
	{
		for (size_t y = 0; y <= detail; y++)
		{
			glm::vec4 vertr;
			float rx = (float)(x * 2);
			float ry = (float)(y * 2);
			vertr.x = (rx * step) - 1.0f;
			vertr.y = (ry * step) - 1.0f;
			vertr.w = 1.0f;
			vertr = tform * vertr;
			glm::vec3 vert = glm::vec3(vertr.x, vertr.y, vertr.z);
			out.push_back(vert);
		}
	}

	return out;
}


void DCubeSphere::bend_cube_face(std::vector<glm::vec3>* points)
{
	for (size_t i = 0; i < points->size(); i++)
	{
		/*glm::vec3 p = glm::normalize(points->at(i));
		(*points)[i] = p;*/

		glm::vec3 p = points->at(i);
		float x2 = p.x * p.x;
		float y2 = p.y * p.y;
		float z2 = p.z * p.z;

		p.x = p.x * sqrt(1.f - y2 / 2.f - z2 / 2.f + y2 * z2 / 3.f);
		p.y = p.y * sqrt(1.f - x2 / 2.f - z2 / 2.f + x2 * z2 / 3.f);
		p.z = p.z * sqrt(1.f - x2 / 2.f - y2 / 2.f + x2 * y2 / 3.f);

		(*points)[i] = p;
	}
}

void DCubeSphere::generate_face(glm::vec3 trans, glm::vec3 rot, Mesh* target)
{
	size_t detail = 10;

	glm::mat4 tform = glm::mat4();
	tform = glm::translate(tform, trans);
	if (rot != glm::vec3(0, 0, 0))
	{
		tform = tform * glm::rotate(glm::radians(90.0f), rot);
	}
	auto verts = make_cube_face(detail, tform);
	bend_cube_face(&verts);

	/*for (size_t i = 0; i < verts.size(); i++)
	{
		Vertex vert; vert.pos = verts[i];
		vert.col = vert.pos;
		target->vertices.push_back(vert);
	}*/

	for (size_t i = 0; i < verts.size() - (detail + 1); i++)
	{
		Vertex v;
		v.col = verts[i];

		if ((i + 1) % (detail + 1) != 0)
		{
			// We are not a rightmost-vertex, do triangles
			v.pos = verts[i];
			target->vertices.push_back(v);
			v.pos = verts[i + 1];
			target->vertices.push_back(v);
			v.pos = verts[i + detail + 1];
			target->vertices.push_back(v);
			v.pos = verts[i + 1];
			target->vertices.push_back(v);
			v.pos = verts[i + detail + 2];
			target->vertices.push_back(v);
			v.pos = verts[i + detail + 1];
			target->vertices.push_back(v);
		}
	}

	target->build_array();
	target->upload();
}

void DCubeSphere::generate_base()
{
	generate_face(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), &rootx.mesh);
	generate_face(glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), &rooty.mesh);
	generate_face(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), &rootz.mesh);
	generate_face(glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), &rootmx.mesh);
	generate_face(glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), &rootmy.mesh);
	generate_face(glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), &rootmz.mesh);
}

void DCubeSphere::draw(glm::mat4 view, glm::mat4 proj)
{
	g_shader->use();

	// Set uniforms
	g_shader->setmat4("model", glm::mat4());
	g_shader->setmat4("view", view);
	g_shader->setmat4("proj", proj);

	// Draw
	glBindVertexArray(rootx.mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, rootx.mesh.vertices.size());
	glBindVertexArray(rooty.mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, rooty.mesh.vertices.size());
	glBindVertexArray(rootz.mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, rootz.mesh.vertices.size());
	glBindVertexArray(rootmx.mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, rootmx.mesh.vertices.size());
	glBindVertexArray(rootmy.mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, rootmy.mesh.vertices.size());
	glBindVertexArray(rootmz.mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, rootmz.mesh.vertices.size());

	Mesh axis;
	Vertex v;
	v.pos = glm::vec3(0, 0, 0);
	v.col = glm::vec3(1, 0, 0);
	axis.vertices.push_back(v);
	v.pos = glm::vec3(100, 0, 0);
	axis.vertices.push_back(v);
	v.pos = glm::vec3(0, 0, 0);
	v.col = glm::vec3(0, 1, 0);
	axis.vertices.push_back(v);
	v.pos = glm::vec3(0, 100, 0);
	axis.vertices.push_back(v);
	v.pos = glm::vec3(0, 0, 0);
	v.col = glm::vec3(0, 0, 1);
	axis.vertices.push_back(v);
	v.pos = glm::vec3(0, 0, 100);
	axis.vertices.push_back(v);

	axis.build_array();
	axis.upload();

	glBindVertexArray(axis.vao);
	glDrawArrays(GL_LINES, 0, axis.vertices.size());
}


DCubeSphere::DCubeSphere()
{
}


DCubeSphere::~DCubeSphere()
{
}
