#include "planet_tile_server.h"
#include "surface_provider.h"


PlanetTile* PlanetTileServer::load(PlanetTilePath path, bool low_up, bool low_right, bool low_down, bool low_left, bool now)
{
	auto it = tiles.find(path);
	PlanetTile* out;

	if (it != tiles.end())
	{
		out = it->second;
	}
	else
	{
		PlanetTile* nTile = new PlanetTile(path, verticesPerSide, *planet);
		tiles[path] = nTile;
		out = nTile;
	}

	out->users++;
	out->needs_lower[0] = low_up;
	out->needs_lower[1] = low_right;
	out->needs_lower[2] = low_down;
	out->needs_lower[3] = low_left;

	if (now)
	{
		if (!out->isUploaded())
		{
			out->upload();
		}
	}

	return out;
}

void PlanetTileServer::unload(PlanetTilePath path, bool unload_now)
{
	auto it = tiles.find(path);
	if (it != tiles.end())
	{
		it->second->users--;

		if (unload_now)
		{
			if (it->second->users <= 0)
			{
				it->second->unload();

				if (it->second->path.get_depth() >= minDepthToUnload)
				{
					delete it->second;
					tiles.erase(it);
				}
			}
		}
	}
}

void PlanetTileServer::unload_unused()
{
	std::vector<PlanetTilePath> toDelete;

	for (auto const& pair : tiles)
	{
		if (pair.second->users <= 0)
		{
			if (pair.second->isUploaded())
			{
				pair.second->unload();
			}

			if (pair.second->path.get_depth() >= minDepthToUnload)
			{
				toDelete.push_back(pair.first);
			}
		}
	}

	for (size_t i = 0; i < toDelete.size(); i++)
	{
		delete tiles[toDelete[i]];
		tiles.erase(toDelete[i]);
	}
}

void PlanetTileServer::upload_used()
{
	size_t i = 0;

	for (auto const& pair : tiles)
	{
		if (pair.second->users > 0)
		{
			if (!pair.second->isUploaded())
			{
				pair.second->upload();
				i++;
			}
		}
	}
}

PlanetTileServer::PlanetTileServer(Planet* planet)
{
	this->planet = planet;
}


PlanetTileServer::~PlanetTileServer()
{
}

double sizeAtPathDepth(size_t depth)
{
	return 1.0 / pow(2, depth);
}

size_t PlanetTilePath::get_depth()
{
	return path.size();
}

glm::dvec2 PlanetTilePath::get_min()
{
	glm::dvec2 out = glm::dvec2(0.0, 0.0);

	for (size_t i = 0; i < path.size(); i++)
	{
		double size = sizeAtPathDepth(i + 1);

		if (path[i] == QuadTreeNode::NORTH_WEST)
		{
			// Nothing
		}
		else if (path[i] == QuadTreeNode::NORTH_EAST)
		{
			out.x += size;
		}
		else if (path[i] == QuadTreeNode::SOUTH_WEST)
		{
			out.y += size;
		}
		else
		{
			out.x += size;
			out.y += size;
		}
	}

	return out;
}

double PlanetTilePath::getSize()
{
	return sizeAtPathDepth(path.size());
}

bool arePathsEqual(const std::vector<QuadTreeNode::QuadTreeQuadrant>& a, const std::vector<QuadTreeNode::QuadTreeQuadrant>& b)
{
	if (a.size() == b.size())
	{
		for (size_t i = 0; i < a.size(); i++)
		{
			if (a[i] != b[i])
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool operator==(const PlanetTilePath& a, const PlanetTilePath& b)
{
	return a.side == b.side && arePathsEqual(a.path, b.path);
}


glm::vec3 PlanetTilePath::get_tile_rotation()
{
	// Tiles look by default into the positive Z so...
	float rot = glm::radians(90.0f);

	if (side == PlanetTilePath::PX)
	{
		return glm::vec3(0.0f, rot, 0.0f);
	}
	else if (side == PlanetTilePath::NX)
	{
		return glm::vec3(0.0f, -rot, 0.0f);
	}
	else if (side == PlanetTilePath::PY)
	{
		return glm::vec3(rot, 0.0f, 0.0f);
	}
	else if (side == PlanetTilePath::NY)
	{
		return glm::vec3(-rot, 0.0f, 0.0f);
	}
	else if (side == PlanetTilePath::PZ)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}
	else if (side == PlanetTilePath::NZ)
	{
		return glm::vec3(0.0f, rot * 2.0f, 0.0f);
	}
}

glm::vec3 PlanetTilePath::get_tile_postrotation()
{
	float r_90 = glm::radians(90.0f);

	if (side == PlanetTilePath::PX)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}
	else if (side == PlanetTilePath::NX)
	{
		return glm::vec3(0.0f, r_90 * 2.0f, 0.0f);
	}
	else if (side == PlanetTilePath::PY)
	{
		return glm::vec3(r_90 * 2.0f, -r_90, 0.0f);
	}
	else if (side == PlanetTilePath::NY)
	{
		return glm::vec3(r_90 * 2.0f, -r_90, 0.0f);
	}
	else if (side == PlanetTilePath::PZ)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}
	else if (side == PlanetTilePath::NZ)
	{
		return glm::vec3(0.0f, 0.0f, 0.0f);
	}
}

glm::vec3 PlanetTilePath::get_tile_origin()
{
	return glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::vec3 PlanetTilePath::get_tile_translation(bool get_spheric)
{
	glm::vec2 deviation = glm::vec2((get_min().x - 0.5f) * 2.0f, (get_min().y - 0.5f) * 2.0f);
	//deviation += path.getSize() / 2.0f;

	glm::vec3 cubic;

	if (side == PlanetTilePath::PX)
	{
		cubic = glm::vec3(1.0f, -deviation.y, -deviation.x);
	}
	else if (side == PlanetTilePath::NX)
	{
		cubic = glm::vec3(1.0f, deviation.y, deviation.x);
	}
	else if (side == PlanetTilePath::PY)
	{
		cubic = glm::vec3(deviation.x, 1.0f, deviation.y);
	}
	else if (side == PlanetTilePath::NY)
	{
		cubic = glm::vec3(deviation.x, 1.0f, deviation.y);
	}
	else if (side == PlanetTilePath::PZ)
	{
		cubic = glm::vec3(deviation.x, -deviation.y, 1.0f);
	}
	else if (side == PlanetTilePath::NZ)
	{
		cubic = glm::vec3(-deviation.x, -deviation.y, -1.0f);
	}

	glm::vec3 spheric = MathUtil::cube_to_sphere(cubic);

	if (get_spheric)
	{
		return spheric;
	}

	return cubic;
}

glm::vec3 PlanetTilePath::get_tile_scale()
{
	float scale = getSize() * 2.0f;

	if (side == PlanetTilePath::PX)
	{
		return glm::vec3(scale, -scale, scale);
	}
	else if (side == PlanetTilePath::NX)
	{
		return glm::vec3(scale, scale, scale);
	}
	else if (side == PlanetTilePath::PY)
	{
		return glm::vec3(scale, scale, scale);
	}
	else if (side == PlanetTilePath::NY)
	{
		return glm::vec3(scale, -scale, scale);
	}
	else if (side == PlanetTilePath::PZ)
	{
		return glm::vec3(scale, -scale, scale);
	}
	else if (side == PlanetTilePath::NZ)
	{
		return glm::vec3(scale, -scale, scale);
	}
}

glm::vec3 PlanetTilePath::get_tile_postscale()
{
	if (side == PlanetTilePath::PY)
	{
		return glm::vec3(1.0f, -1.0f, 1.0f);
	}
	else if (side == PlanetTilePath::NY)
	{
		return glm::vec3(1.0f, 1.0f, -1.0f);
	}
	else if (side == PlanetTilePath::NX)
	{
		return glm::vec3(1.0f, 1.0f, -1.0f);
	}

	return glm::vec3(1.0f, 1.0f, 1.0f);
}

glm::mat4 PlanetTilePath::get_model_matrix()
{
	glm::mat4 translation_mat = glm::translate(glm::mat4(), get_tile_translation(false));
	glm::mat4 scale_mat = glm::scale(glm::mat4(), get_tile_scale());
	glm::mat4 origin_mat = glm::translate(glm::mat4(), get_tile_origin());
	glm::mat4 rotation_mat = glm::toMat4(glm::quat(get_tile_rotation()));
	glm::mat4 postscale_mat = glm::scale(glm::mat4(), get_tile_postscale());
	glm::mat4 postrotate_mat = glm::toMat4(glm::quat(get_tile_postrotation()));

	glm::mat4 model = postrotate_mat * postscale_mat * translation_mat * rotation_mat * scale_mat * origin_mat;

	return model;
}

glm::mat4 PlanetTilePath::get_model_spheric_matrix()
{
	glm::mat4 translation_mat_sph = glm::translate(glm::mat4(), get_tile_translation(true));
	glm::mat4 scale_mat = glm::scale(glm::mat4(), get_tile_scale());
	glm::mat4 origin_mat = glm::translate(glm::mat4(), get_tile_origin());
	glm::mat4 rotation_mat = glm::toMat4(glm::quat(get_tile_rotation()));
	glm::mat4 postscale_mat = glm::scale(glm::mat4(), get_tile_postscale());
	glm::mat4 postrotate_mat = glm::toMat4(glm::quat(get_tile_postrotation()));

	glm::mat4 model_spheric = postrotate_mat * postscale_mat * translation_mat_sph * rotation_mat * scale_mat * origin_mat;

	return model_spheric;
}


glm::vec3 get_real_cubic_pos(glm::vec3 vert, glm::mat4 transform)
{
	return transform * glm::vec4(vert, 1.0f);
}

void PlanetTile::generate_vertex(size_t ix, size_t iy, size_t vertCount, size_t vertCountHeight,
	std::vector<float>& heights, glm::mat4 model,
	glm::mat4 inverse_model_spheric, size_t index, std::vector<float>& target, float size)
{
	float x = (float)ix / ((float)vertCount - 1);
	float y = (float)iy / ((float)vertCount - 1);
	// We have an extra height layer
	float height = heights[(iy + 1) * vertCountHeight + (ix + 1)];

	// Obtain normals from heightmap
	float uheight = heights[(iy + 0) * vertCountHeight + (ix + 1)];
	float rheight = heights[(iy + 1) * vertCountHeight + (ix + 2)];
	float dheight = heights[(iy + 2) * vertCountHeight + (ix + 1)];
	float lheight = heights[(iy + 1) * vertCountHeight + (ix + 0)];

	glm::vec3 va = glm::normalize(glm::vec3((size / vertCountHeight) * 2.0f, 0.0f, rheight - lheight));
	glm::vec3 vb = glm::normalize(glm::vec3(0.0f, (size / vertCountHeight) * 2.0f, uheight - dheight));
	glm::vec3 height_normal = glm::cross(va, vb);

	Vertex out;

	glm::vec3 in_tile = glm::vec3(x, y, 0.0f);

	// Get the absolute [-1, 1] position of the vertex
	glm::vec3 world_pos_cubic = get_real_cubic_pos(in_tile, model);
	glm::vec3 world_pos_spheric = MathUtil::cube_to_sphere(world_pos_cubic);

	glm::vec3 tile_normal = MathUtil::cube_to_sphere(world_pos_spheric);

	world_pos_spheric += glm::normalize(world_pos_spheric) * height;



	// https://blog.selfshadow.com/publications/blending-in-detail/
	glm::vec2 n1xy = glm::vec2(tile_normal.x, tile_normal.y);
	glm::vec2 n2xy = glm::vec2(height_normal.x, height_normal.y);

	glm::vec3 combined_normal = glm::normalize(glm::vec3(n1xy + n2xy, tile_normal.z * height_normal.z));


	out.pos = inverse_model_spheric * glm::vec4(world_pos_spheric, 1.0f);
	out.nrm = combined_normal;

	target[index + 0] = out.pos.x;
	target[index + 1] = out.pos.y;
	target[index + 2] = out.pos.z;
	target[index + 3] = out.nrm.x;
	target[index + 4] = out.nrm.y;
	target[index + 5] = out.nrm.z;
	target[index + 6] = out.uv.x;
	target[index + 7] = out.uv.y;
}


PlanetTile::PlanetTile(PlanetTilePath nPath, size_t vertCount, const Planet& planet) : path(nPath.path, nPath.side), planet(planet)
{
	this->users = 0;
	this->verts = std::vector<float>();
	this->indices = std::vector<uint16_t>();

	this->vert_count = vertCount;

	generate();

	vao = 0; vbo = 0; ebo = 0;
}


bool PlanetTile::isUploaded()
{
	// Just checking one is enough
	return vao != 0;
}

void do_attributes()
{
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// tex
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

}

void PlanetTile::upload()
{
	if (isUploaded())
	{
		spdlog::get("OSP")->warn("Tried to upload an already uploaded PlanetTile");
		return;
	}


	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(4, tolower_vao);
	glGenBuffers(4, tolower_ebo);
	glGenVertexArrays(4, tosame_vao);
	glGenBuffers(4, tosame_ebo);


	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * verts.size(), verts.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);

	do_attributes();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);


	for (size_t i = 0; i < 4; i++)
	{
		glBindVertexArray(tolower_vao[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tolower_ebo[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tolower[i][0]) * tolower[i].size(), tolower[i].data(), GL_STATIC_DRAW);

		do_attributes();

		glBindVertexArray(tosame_vao[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tosame_ebo[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tosame[i][0]) * tosame[i].size(), tosame[i].data(), GL_STATIC_DRAW);

		do_attributes();
	}
}

void PlanetTile::unload()
{
	if (!isUploaded())
	{
		spdlog::get("OSP")->warn("Tried to unload an unloaded PlanetTile");
		return;
	}

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	glDeleteVertexArrays(4, tosame_vao);
	glDeleteBuffers(4, tosame_ebo);
	glDeleteVertexArrays(4, tolower_vao);
	glDeleteBuffers(4, tolower_ebo);

	for (size_t i = 0; i < 4; i++)
	{
		tosame_vao[i] = 0; tosame_ebo[i] = 0;
		tolower_vao[i] = 0; tosame_vao[i] = 0;
	}

	vao = 0;
	vbo = 0;
	ebo = 0;
}

void PlanetTile::generate()
{
	bool clockwise = false;

	if (path.side == PlanetTilePath::PY ||
		path.side == PlanetTilePath::NY ||
		path.side == PlanetTilePath::NX)
	{
		clockwise = true;
	}


	// Used for normal generation, not for rendering
	std::vector<uint16_t> all_indices;

	// We use only position, normal and texture
	const int FLOATS_PER_VERTEX = 8;

	verts.resize(vert_count * vert_count  * FLOATS_PER_VERTEX);

	glm::mat4 model = path.get_model_matrix();
	glm::mat4 model_spheric = path.get_model_spheric_matrix();
	glm::mat4 inverse_model = glm::inverse(model);
	glm::mat4 inverse_model_spheric = glm::inverse(model_spheric);

	std::vector<float> heights = std::vector<float>();
	heights.resize((vert_count + 2) * (vert_count + 2), 0.0f);

	if (planet.surface_provider != NULL)
	{
		planet.surface_provider->get_heights(path, vert_count, heights, planet);
	}


	size_t vertCountp1 = vert_count;

	// Generate all vertices
	for (size_t iy = 0; iy < vert_count; iy++)
	{
		for (size_t ix = 0; ix < vert_count; ix++)
		{
			size_t idx = iy * vertCountp1 + ix;
			generate_vertex(ix, iy, vert_count, vert_count + 2, heights, model,
				inverse_model_spheric, idx * FLOATS_PER_VERTEX, verts, path.getSize());
		}
	}


	// Generate indices for normal mesh
	for (size_t iy = 0; iy < vert_count - 1; iy++)
	{
		for (size_t ix = 0; ix < vert_count; ix++)
		{
			bool to_render = false;

			if (iy >= 1 && iy < vert_count - 1 && ix >= 1 && ix < vert_count - 1)
			{
				to_render = true;
			}

			if (ix < vert_count - 2)
			{
				if (to_render)
				{
					if (clockwise)
					{
						// Center
						indices.push_back(iy * vert_count + ix);
						// Right
						indices.push_back(iy * vert_count + (ix + 1));
					}
					else
					{
						// Right
						indices.push_back(iy * vert_count + (ix + 1));
						// Center
						indices.push_back(iy * vert_count + ix);
					}

					// Down
					indices.push_back((iy + 1) * vert_count + ix);
				}


				if (clockwise)
				{
					// Center
					all_indices.push_back(iy * vertCountp1 + ix);
					// Right
					all_indices.push_back(iy * vertCountp1 + (ix + 1));
				}
				else
				{
					// Right
					all_indices.push_back(iy * vertCountp1 + (ix + 1));
					// Center
					all_indices.push_back(iy * vertCountp1 + ix);
				}

				// Down
				all_indices.push_back((iy + 1) * vertCountp1 + ix);

			}

			if (ix > 1)
			{
				if (to_render)
				{
					if (clockwise)
					{
						// Center
						indices.push_back(iy * vert_count + ix);
						// Down
						indices.push_back((iy + 1) * vert_count + ix);
					}
					else
					{
						// Down
						indices.push_back((iy + 1) * vert_count + ix);
						// Center
						indices.push_back(iy * vert_count + ix);
					}

					// Down Left
					indices.push_back((iy + 1) * vert_count + (ix - 1));
				}

				if (clockwise)
				{
					// Center
					all_indices.push_back(iy * vertCountp1 + ix);
					// Down
					all_indices.push_back((iy + 1) * vertCountp1 + ix);
				}
				else
				{
					// Down
					all_indices.push_back((iy + 1) * vertCountp1 + ix);
					// Center
					all_indices.push_back(iy * vertCountp1 + ix);
				}

				// Down Left
				all_indices.push_back((iy + 1) * vertCountp1 + (ix - 1));
			}

		}
	}


	// Upper and lower (to same)
	for (size_t ix = 0; ix < vert_count - 1; ix++)
	{
		if (clockwise)
		{
			tosame[0].push_back(0 * vert_count + ix + 0);
			tosame[0].push_back(0 * vert_count + ix + 1);
			tosame[0].push_back(1 * vert_count + ix + 0);

			tosame[0].push_back(0 * vert_count + ix + 1);
			tosame[0].push_back(1 * vert_count + ix + 1);
			tosame[0].push_back(1 * vert_count + ix + 0);

			tosame[2].push_back((vert_count - 2) * vert_count + ix + 0);
			tosame[2].push_back((vert_count - 2) * vert_count + ix + 1);
			tosame[2].push_back((vert_count - 1) * vert_count + ix + 0);

			tosame[2].push_back((vert_count - 2) * vert_count + ix + 1);
			tosame[2].push_back((vert_count - 1) * vert_count + ix + 1);
			tosame[2].push_back((vert_count - 1) * vert_count + ix + 0);
		}
		else
		{
			tosame[0].push_back(0 * vert_count + ix + 1);
			tosame[0].push_back(0 * vert_count + ix + 0);
			tosame[0].push_back(1 * vert_count + ix + 0);

			tosame[0].push_back(1 * vert_count + ix + 1);
			tosame[0].push_back(0 * vert_count + ix + 1);
			tosame[0].push_back(1 * vert_count + ix + 0);

			tosame[2].push_back((vert_count - 2) * vert_count + ix + 1);
			tosame[2].push_back((vert_count - 2) * vert_count + ix + 0);
			tosame[2].push_back((vert_count - 1) * vert_count + ix + 0);

			tosame[2].push_back((vert_count - 1) * vert_count + ix + 1);
			tosame[2].push_back((vert_count - 2) * vert_count + ix + 1);
			tosame[2].push_back((vert_count - 1) * vert_count + ix + 0);
		}



	}

	// Left and right (to same)
	for (size_t iy = 0; iy < vert_count - 1; iy++)
	{
		if (clockwise)
		{
			tosame[3].push_back((iy + 0) * vert_count + 0);
			tosame[3].push_back((iy + 0) * vert_count + 1);
			tosame[3].push_back((iy + 1) * vert_count + 0);

			tosame[3].push_back((iy + 0) * vert_count + 1);
			tosame[3].push_back((iy + 1) * vert_count + 1);
			tosame[3].push_back((iy + 1) * vert_count + 0);

			tosame[1].push_back((iy + 0) * vert_count + vert_count - 2);
			tosame[1].push_back((iy + 0) * vert_count + vert_count - 1);
			tosame[1].push_back((iy + 1) * vert_count + vert_count - 2);

			tosame[1].push_back((iy + 0) * vert_count + vert_count - 1);
			tosame[1].push_back((iy + 1) * vert_count + vert_count - 1);
			tosame[1].push_back((iy + 1) * vert_count + vert_count - 2);
		}
		else
		{
			tosame[3].push_back((iy + 0) * vert_count + 1);
			tosame[3].push_back((iy + 0) * vert_count + 0);
			tosame[3].push_back((iy + 1) * vert_count + 0);

			tosame[3].push_back((iy + 1) * vert_count + 1);
			tosame[3].push_back((iy + 0) * vert_count + 1);
			tosame[3].push_back((iy + 1) * vert_count + 0);

			tosame[1].push_back((iy + 0) * vert_count + vert_count - 1);
			tosame[1].push_back((iy + 0) * vert_count + vert_count - 2);
			tosame[1].push_back((iy + 1) * vert_count + vert_count - 2);

			tosame[1].push_back((iy + 1) * vert_count + vert_count - 1);
			tosame[1].push_back((iy + 0) * vert_count + vert_count - 1);
			tosame[1].push_back((iy + 1) * vert_count + vert_count - 2);
		}

	}

	// Upper and lower (to lower)
	for (size_t ix = 0; ix < vert_count - 2; ix += 2)
	{
		if (clockwise)
		{
			tolower[0].push_back(0 * vert_count + ix + 0);
			tolower[0].push_back(1 * vert_count + ix + 1);
			tolower[0].push_back(1 * vert_count + ix + 0);

			tolower[0].push_back(0 * vert_count + ix + 0);
			tolower[0].push_back(0 * vert_count + ix + 2);
			tolower[0].push_back(1 * vert_count + ix + 1);

			tolower[0].push_back(0 * vert_count + ix + 2);
			tolower[0].push_back(1 * vert_count + ix + 2);
			tolower[0].push_back(1 * vert_count + ix + 1);

			tolower[2].push_back((vert_count - 2) * vert_count + ix + 0);
			tolower[2].push_back((vert_count - 2) * vert_count + ix + 1);
			tolower[2].push_back((vert_count - 1) * vert_count + ix + 0);

			tolower[2].push_back((vert_count - 1) * vert_count + ix + 0);
			tolower[2].push_back((vert_count - 2) * vert_count + ix + 1);
			tolower[2].push_back((vert_count - 1) * vert_count + ix + 2);

			tolower[2].push_back((vert_count - 2) * vert_count + ix + 1);
			tolower[2].push_back((vert_count - 2) * vert_count + ix + 2);
			tolower[2].push_back((vert_count - 1) * vert_count + ix + 2);
		}
		else
		{
			tolower[0].push_back(1 * vert_count + ix + 1);
			tolower[0].push_back(0 * vert_count + ix + 0);
			tolower[0].push_back(1 * vert_count + ix + 0);

			tolower[0].push_back(0 * vert_count + ix + 2);
			tolower[0].push_back(0 * vert_count + ix + 0);
			tolower[0].push_back(1 * vert_count + ix + 1);

			tolower[0].push_back(1 * vert_count + ix + 2);
			tolower[0].push_back(0 * vert_count + ix + 2);
			tolower[0].push_back(1 * vert_count + ix + 1);

			tolower[2].push_back((vert_count - 2) * vert_count + ix + 1);
			tolower[2].push_back((vert_count - 2) * vert_count + ix + 0);
			tolower[2].push_back((vert_count - 1) * vert_count + ix + 0);

			tolower[2].push_back((vert_count - 2) * vert_count + ix + 1);
			tolower[2].push_back((vert_count - 1) * vert_count + ix + 0);
			tolower[2].push_back((vert_count - 1) * vert_count + ix + 2);

			tolower[2].push_back((vert_count - 2) * vert_count + ix + 2);
			tolower[2].push_back((vert_count - 2) * vert_count + ix + 1);
			tolower[2].push_back((vert_count - 1) * vert_count + ix + 2);
		}

	}

	// Left and Right (to lower)
	for (size_t iy = 0; iy < vert_count - 2; iy += 2)
	{
		if (clockwise)
		{
			tolower[3].push_back((iy + 0) * vert_count + 0);
			tolower[3].push_back((iy + 0) * vert_count + 1);
			tolower[3].push_back((iy + 1) * vert_count + 1);

			tolower[3].push_back((iy + 1) * vert_count + 1);
			tolower[3].push_back((iy + 2) * vert_count + 1);
			tolower[3].push_back((iy + 2) * vert_count + 0);

			tolower[3].push_back((iy + 0) * vert_count + 0);
			tolower[3].push_back((iy + 1) * vert_count + 1);
			tolower[3].push_back((iy + 2) * vert_count + 0);

			tolower[1].push_back((iy + 0) * vert_count + vert_count - 2);
			tolower[1].push_back((iy + 0) * vert_count + vert_count - 1);
			tolower[1].push_back((iy + 1) * vert_count + vert_count - 2);

			tolower[1].push_back((iy + 1) * vert_count + vert_count - 2);
			tolower[1].push_back((iy + 2) * vert_count + vert_count - 1);
			tolower[1].push_back((iy + 2) * vert_count + vert_count - 2);

			tolower[1].push_back((iy + 0) * vert_count + vert_count - 1);
			tolower[1].push_back((iy + 2) * vert_count + vert_count - 1);
			tolower[1].push_back((iy + 1) * vert_count + vert_count - 2);
		}
		else
		{
			tolower[3].push_back((iy + 0) * vert_count + 1);
			tolower[3].push_back((iy + 0) * vert_count + 0);
			tolower[3].push_back((iy + 1) * vert_count + 1);

			tolower[3].push_back((iy + 2) * vert_count + 1);
			tolower[3].push_back((iy + 1) * vert_count + 1);
			tolower[3].push_back((iy + 2) * vert_count + 0);

			tolower[3].push_back((iy + 1) * vert_count + 1);
			tolower[3].push_back((iy + 0) * vert_count + 0);
			tolower[3].push_back((iy + 2) * vert_count + 0);

			tolower[1].push_back((iy + 0) * vert_count + vert_count - 1);
			tolower[1].push_back((iy + 0) * vert_count + vert_count - 2);
			tolower[1].push_back((iy + 1) * vert_count + vert_count - 2);

			tolower[1].push_back((iy + 2) * vert_count + vert_count - 1);
			tolower[1].push_back((iy + 1) * vert_count + vert_count - 2);
			tolower[1].push_back((iy + 2) * vert_count + vert_count - 2);

			tolower[1].push_back((iy + 2) * vert_count + vert_count - 1);
			tolower[1].push_back((iy + 0) * vert_count + vert_count - 1);
			tolower[1].push_back((iy + 1) * vert_count + vert_count - 2);
		}
	}
}

