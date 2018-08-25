#include "dcubesphere.h"


std::vector<CubeSpherePoint> DCubeSphere::make_cube_face(size_t detail, glm::mat4 tform, Image* img, size_t inv, 
	glm::vec4 our_bounds, glm::vec4 child_bounds)
{
	std::vector<CubeSpherePoint> out;
	float step = 1.0 / (float)detail;

	for (size_t x = 0; x <= detail; x++)
	{
		for (size_t y = 0; y <= detail; y++)
		{
			float px = (float)x / (float)detail;
			float py = (float)y / (float)detail;

			bool inside_our_bounds = px >= our_bounds.x && px <= our_bounds.z && py >= our_bounds.y && py <= our_bounds.w;
			bool inside_child_bounds = px >= child_bounds.x && px <= child_bounds.z && py >= child_bounds.y && py <= child_bounds.w;

			if (inside_our_bounds && !inside_child_bounds)
			{
				Pixel p;
				if (inv == 1)
				{
					p = img->get_pixel(1.0f - ((float)x / (float)detail), 1.0f - ((float)y / (float)detail));
				}
				else if (inv == 0)
				{
					p = img->get_pixel((float)x / (float)detail, 1.0f - ((float)y / (float)detail));
				}
				else if (inv == 2)
				{
					p = img->get_pixel((float)x / (float)detail, ((float)y / (float)detail));
				}

				float h = ((float)p.r / 255.0f);
				h *= heightmap_power;
				h += 1.0f;

				glm::vec4 vertr;
				float rx = (float)(x * 2);
				float ry = (float)(y * 2);
				vertr.x = (rx * step) - 1.0f;
				vertr.y = (ry * step) - 1.0f;
				vertr.w = 1.0f;
				vertr = tform * vertr;
				glm::vec3 vert = glm::vec3(vertr.x, vertr.y, vertr.z);
				out.push_back(CubeSpherePoint(vert, h));
			}
			else
			{
				glm::vec3 vert = glm::vec3(0, 0, 0);
				out.push_back(CubeSpherePoint(vert, 0.0f));
			}
		}
	}

	return out;
}


void DCubeSphere::bend_cube_face(std::vector<CubeSpherePoint>* points, bool adv_mapping)
{
	for (size_t i = 0; i < points->size(); i++)
	{
		glm::vec3 n = points->at(i).p;
		if (n != glm::vec3(0, 0, 0))
		{

			if (adv_mapping)
			{
				float x2 = n.x * n.x;
				float y2 = n.y * n.y;
				float z2 = n.z * n.z;

				n.x = n.x * sqrt(1.f - y2 / 2.f - z2 / 2.f + y2 * z2 / 3.f);
				n.y = n.y * sqrt(1.f - x2 / 2.f - z2 / 2.f + x2 * z2 / 3.f);
				n.z = n.z * sqrt(1.f - x2 / 2.f - y2 / 2.f + x2 * y2 / 3.f);
			}
			else
			{
				n = glm::normalize(points->at(i).p);
			}


			// Apply height
			glm::vec3 p = n * points->at(i).h;

			(*points)[i].p = p;
			(*points)[i].n = n;
		}
	}
}

void DCubeSphere::generate_face(glm::vec3 trans, glm::vec3 rot, Mesh* target, Image* img, size_t inv, 
	glm::vec4 our_bounds, glm::vec4 child_bounds)
{
	size_t detail = 64;

	glm::mat4 tform = glm::mat4();
	tform = glm::translate(tform, trans);
	if (rot != glm::vec3(0, 0, 0))
	{
		tform = tform * glm::rotate(glm::radians(90.0f), rot);
	}
	auto verts = make_cube_face(detail, tform, img, inv, our_bounds, child_bounds);
	bend_cube_face(&verts, false);

	/*for (size_t i = 0; i < verts.size(); i++)
	{
		Vertex vert; vert.pos = verts[i];
		vert.col = vert.pos;
		target->vertices.push_back(vert);
	}*/

	for (size_t i = 0; i < verts.size() - (detail + 1); i++)
	{
		Vertex v;
		float val = (verts[i].h - 1.0f) * (1.0f / heightmap_power);
		v.col = glm::vec3(val, val, val);

		
		if ((i + 1) % (detail + 1) != 0)
		{
			if (verts[i].h <= 0.005 || verts[i + 1].h <= 0.005 || 
				verts[i + detail + 1].h <= 0.005 || verts[i + detail + 2].h <= 0.005)
			{
				// Do nothing or maybe add a little "gap" vertex
			}
			else
			{
				// We are not a rightmost-vertex, do triangles
				v.pos = verts[i].p;
				target->vertices.push_back(v);
				v.pos = verts[i + 1].p;
				target->vertices.push_back(v);
				v.pos = verts[i + detail + 1].p;
				target->vertices.push_back(v);
				v.pos = verts[i + 1].p;
				target->vertices.push_back(v);
				v.pos = verts[i + detail + 2].p;
				target->vertices.push_back(v);
				v.pos = verts[i + detail + 1].p;
				target->vertices.push_back(v);
			}
		}
	}

	target->build_array();
	target->upload();
}

void DCubeSphere::generate_base()
{
	assert(cubemap.size() != 0);

	glm::vec4 full = glm::vec4(0.0, 0.0, 1.0, 1.0);
	glm::vec4 null = glm::vec4(0.0, 0.0, 0.0, 0.0);

	generate_face(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), &rootx.mesh, &cubemap[0], 0, full, null);
	generate_face(glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), &rooty.mesh, &cubemap[2], 2, full, null);
	generate_face(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), &rootz.mesh, &cubemap[4], 0, full, null);
	generate_face(glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), &rootmx.mesh, &cubemap[1], 1, full, null);
	generate_face(glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), &rootmy.mesh, &cubemap[3], 0, full, null);
	generate_face(glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), &rootmz.mesh, &cubemap[5], 1, full, null);
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


void DCubeSphere::load_cubemap(std::string base_path)
{
	if (base_path[base_path.size() - 1] != '/')
	{
		base_path += '/';
	}

	Image px = Image::from_file(base_path + "px.png");
	cubemap.push_back(px);
	Image nx = Image::from_file(base_path + "nx.png");
	cubemap.push_back(nx);
	Image py = Image::from_file(base_path + "py.png");
	cubemap.push_back(py);
	Image ny = Image::from_file(base_path + "ny.png");
	cubemap.push_back(ny);
	Image pz = Image::from_file(base_path + "pz.png");
	cubemap.push_back(pz);
	Image nz = Image::from_file(base_path + "nz.png");
	cubemap.push_back(nz);

}

void DCubeSphere::generate(CubeSphereNode* target)
{

}

DCubeSphere::DCubeSphere()
{
}


DCubeSphere::~DCubeSphere()
{
}
