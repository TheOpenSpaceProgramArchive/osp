#include "dcubesphere.h"


std::vector<CubeSpherePoint> DCubeSphere::make_cube_face(size_t detail, glm::mat4 tform, Image* img, size_t inv, 
	glm::vec4 our_bounds, glm::vec4 child_bounds, float hpower)
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
				h *= hpower;
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
				out.push_back(CubeSpherePoint(vert, -100.0f));
			}
		}
	}

	return out;
}


void DCubeSphere::bend_cube_face(std::vector<CubeSpherePoint>* points, float adv_map_mult)
{
	for (size_t i = 0; i < points->size(); i++)
	{
		glm::vec3 n = points->at(i).p;
		float dist_to_edge = glm::length(n) * adv_map_mult;
		dist_to_edge = dist_to_edge * dist_to_edge;
		dist_to_edge = std::min(1.0f, dist_to_edge);

		if (n != glm::vec3(0, 0, 0))
		{
			glm::vec3 adv_n = n;
			glm::vec3 nor_n = n;

			float x2 = adv_n.x * adv_n.x;
			float y2 = adv_n.y * adv_n.y;
			float z2 = adv_n.z * adv_n.z;

			adv_n.x = adv_n.x * sqrtf(1.f - (y2 / 2.f) - (z2 / 2.f) + ((y2 * z2) / 3.f));
			adv_n.y = adv_n.y * sqrtf(1.f - (x2 / 2.f) - (z2 / 2.f) + ((x2 * z2) / 3.f));
			adv_n.z = adv_n.z * sqrtf(1.f - (x2 / 2.f) - (y2 / 2.f) + ((x2 * y2) / 3.f));
			nor_n = glm::normalize(points->at(i).p);

			glm::vec3 n = glm::lerp(nor_n, adv_n, dist_to_edge);
			//(*points)[i].h = (dist_to_edge * 0.05f) + 1.0f;
			//glm::vec3 n = nor_n;

			// Apply height
			glm::vec3 p = n * points->at(i).h;

			(*points)[i].p = p;
			(*points)[i].n = n;
		}
	}
}

void DCubeSphere::generate_face(glm::vec3 trans, glm::vec3 rot, Mesh* target, Image* img, size_t inv, 
	glm::vec4 our_bounds, glm::vec4 child_bounds, bool flip_normal, size_t detail)
{

	glm::mat4 tform = glm::mat4();
	tform = glm::translate(tform, trans);
	if (rot != glm::vec3(0, 0, 0))
	{
		tform = tform * glm::rotate(glm::radians(90.0f), rot);
	}
	auto verts = make_cube_face(detail, tform, img, inv, our_bounds, child_bounds, heightmap_power);
	bend_cube_face(&verts, 0.333f);

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
			if (verts[i].h <= -0.5 || verts[i + 1].h <= -0.5 ||
				verts[i + detail + 1].h <= -0.5 || verts[i + detail + 2].h <= -0.5)
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

	target->generate_normals(false, flip_normal);

	target->build_array();
	target->upload();
}

void DCubeSphere::generate_base()
{
	assert(cubemap.size() != 0);

	glm::vec4 full = glm::vec4(0.0, 0.0, 1.0, 1.0);
	glm::vec4 null = glm::vec4(-1.0, -1.0, -1.0, -1.0);

	generate_face(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), &rootx.mesh, &cubemap[0], 0, full, null, true, 96);
	generate_face(glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), &rooty.mesh, &cubemap[2], 2, full, null, false, 96);
	generate_face(glm::vec3(0, 0, 1), glm::vec3(0, 0, 0), &rootz.mesh, &cubemap[4], 0, full, null, true, 96);
	generate_face(glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), &rootmx.mesh, &cubemap[1], 1, full, null, false, 96);
	generate_face(glm::vec3(0, -1, 0), glm::vec3(1, 0, 0), &rootmy.mesh, &cubemap[3], 0, full, null, true, 96);
	generate_face(glm::vec3(0, 0, -1), glm::vec3(0, 0, 0), &rootmz.mesh, &cubemap[5], 1, full, null, false, 96);
}

void DCubeSphere::draw(glm::mat4 view, glm::mat4 proj)
{
	g_shader->use();

	// Set uniforms
	g_shader->setmat4("model", model);
	g_shader->setmat4("view", view);
	g_shader->setmat4("proj", proj);

	// Draw
	rootx.draw_recursive();
	rooty.draw_recursive();
	rootz.draw_recursive();
	rootmx.draw_recursive();
	rootmy.draw_recursive();
	rootmz.draw_recursive();

	// Draw debug axis
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


void DCubeSphere::update(glm::vec3 p)
{

	if (require_upload.size() > 0)
	{
		for (size_t i = 0; i < require_upload.size(); i++)
		{
			if (!require_upload[i]->being_worked)
			{
				require_upload[i]->mesh.upload(true);
			}
		}

		require_upload.clear();
	}

	update_count++;
	if (update_count >= 10)
	{
		rootx.apply_viewpoint(p);
		rooty.apply_viewpoint(p);
		rootz.apply_viewpoint(p);
		rootmx.apply_viewpoint(p);
		rootmy.apply_viewpoint(p);
		rootmz.apply_viewpoint(p);
		update_count = 0;
	}
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
	target->being_worked = true;

	target->mesh.vertices.clear();

	if (cubemap.size() == 0)
	{
		spdlog::get("OSP")->error("Cubemap not loaded and tried to generate!");
	}

	glm::vec3 trans, rot;
	Image* img = NULL;
	bool flip_normal;
	glm::vec4 child_bounds = glm::vec4(-1.0, -1.0, -1.0, -1.0);
	size_t inv = 0;

	if (target->child != NULL)
	{
		child_bounds = target->child->bounds;
	}

	// Obtain hard-coded values for the different faces
	if (target->face == CubeSphereNode::PX) 
	{ 
		trans = glm::vec3(1, 0, 0); rot = glm::vec3(0, 1, 0); img = &cubemap[0]; flip_normal = true; inv = 0;
	}
	else if (target->face == CubeSphereNode::PY)
	{
		trans = glm::vec3(0, 1, 0); rot = glm::vec3(1, 0, 0); img = &cubemap[2]; flip_normal = false; inv = 2;
	}
	else if (target->face == CubeSphereNode::PZ)
	{
		trans = glm::vec3(0, 0, 1); rot = glm::vec3(0, 0, 0); img = &cubemap[4]; flip_normal = true; inv = 0;
	}
	else if (target->face == CubeSphereNode::NX)
	{
		trans = glm::vec3(-1, 0, 0); rot = glm::vec3(0, 1, 0); img = &cubemap[1]; flip_normal = false; inv = 1;
	}
	else if (target->face == CubeSphereNode::NY)
	{
		trans = glm::vec3(0, -1, 0); rot = glm::vec3(1, 0, 0); img = &cubemap[3]; flip_normal = true; inv = 0;
	}
	else if (target->face == CubeSphereNode::NZ)
	{
		trans = glm::vec3(0, 0, -1); rot = glm::vec3(0, 0, 0); img = &cubemap[5]; flip_normal = false; inv = 1;
	}

	generate_face(trans, rot, &target->mesh, img, inv, target->bounds, child_bounds, flip_normal, target->detail);

	target->mesh.build_array();

	target->being_worked = false;
}

void DCubeSphere::launch_worker()
{
	worker_run = true;
	worker = std::thread(cubesphere_worker, this);
}

DCubeSphere::DCubeSphere()
{
	rootx.dirty = true; rootx.face = CubeSphereNode::PX;
	rooty.dirty = true; rooty.face = CubeSphereNode::PY;
	rootz.dirty = true; rootz.face = CubeSphereNode::PZ;
	rootmx.dirty = true; rootmx.face = CubeSphereNode::NX;
	rootmy.dirty = true; rootmy.face = CubeSphereNode::NY;
	rootmz.dirty = true; rootmz.face = CubeSphereNode::NZ;
}


DCubeSphere::~DCubeSphere()
{
	worker_run = false;
	worker.join();
}

void CubeSphereNode::draw_recursive()
{
	glBindVertexArray(mesh.vao);
	glDrawArrays(GL_TRIANGLES, 0, mesh.vertex_count);

	if (child != NULL)
	{
		child->draw_recursive();
	}
}

void CubeSphereNode::apply_viewpoint(glm::vec3 p, size_t min_detail)
{
	// Closest point on the unit sphere to p
	glm::vec3 closest = glm::normalize(p);
	if (
		(closest.x >= 0.0f && face == NX) ||
		(closest.x <= 0.0f && face == PX) || 
		(closest.y >= 0.0f && face == NY) ||
		(closest.y <= 0.0f && face == PY) || 
		(closest.z >= 0.0f && face == NZ) ||
		(closest.z <= 0.0f && face == PZ)
		)
	{
		// Oppposite side, reduce quality to minimum
		// delete children
		if (detail != min_detail)
		{
			detail = min_detail;
			dirty = true;
		}
		
		collapse();
	}
	else
	{
		// Find reasonably sized and positioned quad
		float dist = glm::length(p);
		if (dist >= 6.0f)
		{
			// 30 times our radius, minimal quality
			if (detail != min_detail)
			{
				detail = min_detail;
				dirty = true;
			}

			collapse();
		}
		else if (dist >= 4.0f)
		{
			// 15 times our radius, medium quality
			if (detail != min_detail * 2)
			{
				detail = min_detail * 2;
				dirty = true;
			}

			collapse();
		}
		else if (dist >= 2.0f)
		{
			// 5 times our radius, decent quality
			if (detail != min_detail * 3)
			{
				detail = min_detail * 3;
				dirty = true;
			}

			collapse();
		}
		else
		{
			// We need LOD-ing now, but base quality is
			if (detail != min_detail * 3)
			{
				detail = min_detail * 3;
				dirty = true;
			}

			generate_children(p, closest);
		}
	}

}

void CubeSphereNode::generate_children(glm::vec3 p, glm::vec3 surf_p)
{
	float dist = glm::length(p);

}

void CubeSphereNode::collapse()
{
	if (child != NULL)
	{
		dirty = true;

		child->collapse();
		delete child;
		child = NULL;
	}
}

CubeSphereNode* DCubeSphere::get_dirty()
{
	CubeSphereNode* rx = rootx.get_dirty();
	if (rx != NULL)
	{
		return rx;
	}
	CubeSphereNode* ry = rooty.get_dirty();
	if (ry != NULL)
	{
		return ry;
	}
	CubeSphereNode* rz = rootz.get_dirty();
	if (rz != NULL)
	{
		return rz;
	}
	CubeSphereNode* rmx = rootmx.get_dirty();
	if (rmx != NULL)
	{
		return rmx;
	}
	CubeSphereNode* rmy = rootmy.get_dirty();
	if (rmy != NULL)
	{
		return rmy;
	}
	CubeSphereNode* rmz = rootmz.get_dirty();
	if (rmz != NULL)
	{
		return rmz;
	}

	return NULL;
}


CubeSphereNode* CubeSphereNode::get_dirty()
{
	if (this->dirty)
	{
		this->dirty = false;
		return this;
	}

	if (child == NULL)
	{
		return NULL;
	}
	else
	{
		return child->get_dirty();
	}
}

void cubesphere_worker(DCubeSphere* owner)
{
	while (owner->worker_run)
	{
		CubeSphereNode* node = owner->get_dirty();

		if (node != NULL)
		{
			owner->generate(node);
			


			bool found = false;
			for (size_t i = 0; i < owner->require_upload.size(); i++)
			{
				if (owner->require_upload[i] == node)
				{
					found = true;
					break;
				}
			}

			if(!found)
			{ 
				owner->require_upload.push_back(node);
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
