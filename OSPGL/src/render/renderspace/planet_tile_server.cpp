#include "planet_tile_server.h"
#include "surface_provider.h"


PlanetTile* PlanetTileServer::load(PlanetTilePath path, bool low_up, bool low_right, bool low_down, bool low_left)
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


	if (!out->isUploaded())
	{
		out->upload();
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

void PlanetTile::generate_vertex(size_t ix, size_t iy, size_t vertCount, 
	std::vector<float>& heights, glm::mat4 model, 
	glm::mat4 inverse_model_spheric, size_t index)
{
	float x = (float)ix / ((float)vertCount - 1);
	float y = (float)iy / ((float)vertCount - 1);
	float height = heights[iy * vertCount + ix];

	Vertex out;

	glm::vec3 in_tile = glm::vec3(x, y, 0.0f);

	// Get the absolute [-1, 1] position of the vertex
	glm::vec3 world_pos_cubic = get_real_cubic_pos(in_tile, model);
	glm::vec3 world_pos_spheric = MathUtil::cube_to_sphere(world_pos_cubic);
	world_pos_spheric += glm::normalize(world_pos_spheric) * height;

	out.pos = inverse_model_spheric * glm::vec4(world_pos_spheric, 1.0f);
	out.nrm = glm::vec3(0.0f, 0.0f, 0.0f);

	verts[index + 0] = out.pos.x;
	verts[index + 1] = out.pos.y;
	verts[index + 2] = out.pos.z;
	verts[index + 3] = out.nrm.x;
	verts[index + 4] = out.nrm.y;
	verts[index + 5] = out.nrm.z;
	verts[index + 6] = out.uv.x;
	verts[index + 7] = out.uv.y;
}


PlanetTile::PlanetTile(PlanetTilePath nPath, size_t vertCount, const Planet& planet) : path(nPath.path, nPath.side)
{
	bool clockwise = false;

	if (nPath.side == PlanetTilePath::PY || 
		nPath.side == PlanetTilePath::NY || 
		nPath.side == PlanetTilePath::NX)
	{
		clockwise = true;
	}

	this->users = 0;
	this->verts = std::vector<float>();
	this->indices = std::vector<uint16_t>();

	// Used for normal generation, not for rendering
	std::vector<uint16_t> all_indices;
	
	// We use only position, normal and texture
	const int FLOATS_PER_VERTEX = 8;

	verts.resize(vertCount * vertCount * FLOATS_PER_VERTEX + vertCount * 4 * FLOATS_PER_VERTEX);

	glm::mat4 model = nPath.get_model_matrix();
	glm::mat4 model_spheric = nPath.get_model_spheric_matrix();
	glm::mat4 inverse_model = glm::inverse(model);
	glm::mat4 inverse_model_spheric = glm::inverse(model_spheric);

	std::vector<float> heights = std::vector<float>();
	heights.resize(vertCount * vertCount, 0.0f);

	if (planet.surface_provider != NULL)
	{
		planet.surface_provider->get_heights(nPath, vertCount, heights, planet);
	}

	// Generate all vertices
	for (size_t iy = 0; iy < vertCount; iy++)
	{	
		for (size_t ix = 0; ix < vertCount; ix++)
		{
			generate_vertex(ix, iy, vertCount, heights, model, inverse_model_spheric, (iy * vertCount + ix) * FLOATS_PER_VERTEX);
		}
	}

	size_t end_index = vertCount * vertCount;
	size_t hVertCount = vertCount / 2;

	size_t left_corner_idx = end_index;
	size_t right_corner_idx = end_index + hVertCount;

	end_index += vertCount;

	for (size_t ix = 0; ix < vertCount; ix += 2)
	{
		// Up and right corners
		generate_vertex(ix, 0, vertCount, heights, model, inverse_model_spheric, (end_index + ix / 2) * FLOATS_PER_VERTEX);
		generate_vertex(ix, vertCount - 1, vertCount, heights, model, inverse_model_spheric, (end_index + hVertCount + ix / 2) * FLOATS_PER_VERTEX);
	}

	size_t up_corner_idx = end_index;
	size_t down_corner_idx = end_index + hVertCount;

	// Generate indices for normal mesh
	for (size_t iy = 0; iy < vertCount - 1; iy++)
	{
		for (size_t ix = 0; ix < vertCount; ix++)
		{
			bool to_render = false;

			if (iy >= 1 && iy < vertCount - 1 && ix >= 1 && ix < vertCount - 1)
			{
				to_render = true;
			}

			if (ix < vertCount - 2)
			{
				if (to_render)
				{
					if (clockwise)
					{
						// Center
						indices.push_back(iy * vertCount + ix);
						// Right
						indices.push_back(iy * vertCount + (ix + 1));
					}
					else
					{
						// Right
						indices.push_back(iy * vertCount + (ix + 1));
						// Center
						indices.push_back(iy * vertCount + ix);
					}

					// Down
					indices.push_back((iy + 1) * vertCount + ix);
				}
				
			
				if (clockwise)
				{
					// Center
					all_indices.push_back(iy * vertCount + ix);
					// Right
					all_indices.push_back(iy * vertCount + (ix + 1));
				}
				else
				{
					// Right
					all_indices.push_back(iy * vertCount + (ix + 1));
					// Center
					all_indices.push_back(iy * vertCount + ix);
				}

				// Down
				all_indices.push_back((iy + 1) * vertCount + ix);

			}

			if (ix > 1)
			{
				if (to_render)
				{
					if (clockwise)
					{
						// Center
						indices.push_back(iy * vertCount + ix);
						// Down
						indices.push_back((iy + 1) * vertCount + ix);
					}
					else
					{
						// Down
						indices.push_back((iy + 1) * vertCount + ix);
						// Center
						indices.push_back(iy * vertCount + ix);
					}

					// Down Left
					indices.push_back((iy + 1) * vertCount + (ix - 1));
				}

				if (clockwise)
				{
					// Center
					all_indices.push_back(iy * vertCount + ix);
					// Down
					all_indices.push_back((iy + 1) * vertCount + ix);
				}
				else
				{
					// Down
					all_indices.push_back((iy + 1) * vertCount + ix);
					// Center
					all_indices.push_back(iy * vertCount + ix);
				}

				// Down Left
				all_indices.push_back((iy + 1) * vertCount + (ix - 1));
			}
		}
	}

	// Upper and lower (to same)
	for (size_t ix = 0; ix < vertCount - 1; ix++)
	{
		if (clockwise)
		{
			tosame[0].push_back(0 * vertCount + ix + 0);
			tosame[0].push_back(0 * vertCount + ix + 1);
			tosame[0].push_back(1 * vertCount + ix + 0);

			tosame[0].push_back(0 * vertCount + ix + 1);
			tosame[0].push_back(1 * vertCount + ix + 1);
			tosame[0].push_back(1 * vertCount + ix + 0);

			tosame[2].push_back((vertCount - 2) * vertCount + ix + 0);
			tosame[2].push_back((vertCount - 2) * vertCount + ix + 1);
			tosame[2].push_back((vertCount - 1) * vertCount + ix + 0);

			tosame[2].push_back((vertCount - 2) * vertCount + ix + 1);
			tosame[2].push_back((vertCount - 1) * vertCount + ix + 1);
			tosame[2].push_back((vertCount - 1) * vertCount + ix + 0);
		}
		else
		{
			tosame[0].push_back(0 * vertCount + ix + 1);
			tosame[0].push_back(0 * vertCount + ix + 0);
			tosame[0].push_back(1 * vertCount + ix + 0);

			tosame[0].push_back(1 * vertCount + ix + 1);
			tosame[0].push_back(0 * vertCount + ix + 1);
			tosame[0].push_back(1 * vertCount + ix + 0);

			tosame[2].push_back((vertCount - 2) * vertCount + ix + 1);
			tosame[2].push_back((vertCount - 2) * vertCount + ix + 0);
			tosame[2].push_back((vertCount - 1) * vertCount + ix + 0);

			tosame[2].push_back((vertCount - 1) * vertCount + ix + 1);
			tosame[2].push_back((vertCount - 2) * vertCount + ix + 1);
			tosame[2].push_back((vertCount - 1) * vertCount + ix + 0);
		}
	


	}

	// Left and right (to same)
	for (size_t iy = 0; iy < vertCount - 1; iy++)
	{
		if (clockwise)
		{
			tosame[3].push_back((iy + 0) * vertCount + 0);
			tosame[3].push_back((iy + 0) * vertCount + 1);
			tosame[3].push_back((iy + 1) * vertCount + 0);

			tosame[3].push_back((iy + 0) * vertCount + 1);
			tosame[3].push_back((iy + 1) * vertCount + 1);
			tosame[3].push_back((iy + 1) * vertCount + 0);

			tosame[1].push_back((iy + 0) * vertCount + vertCount - 2);
			tosame[1].push_back((iy + 0) * vertCount + vertCount - 1);
			tosame[1].push_back((iy + 1) * vertCount + vertCount - 2);

			tosame[1].push_back((iy + 0) * vertCount + vertCount - 1);
			tosame[1].push_back((iy + 1) * vertCount + vertCount - 1);
			tosame[1].push_back((iy + 1) * vertCount + vertCount - 2);
		}
		else
		{
			tosame[3].push_back((iy + 0) * vertCount + 1);
			tosame[3].push_back((iy + 0) * vertCount + 0);
			tosame[3].push_back((iy + 1) * vertCount + 0);

			tosame[3].push_back((iy + 1) * vertCount + 1);
			tosame[3].push_back((iy + 0) * vertCount + 1);
			tosame[3].push_back((iy + 1) * vertCount + 0);

			tosame[1].push_back((iy + 0) * vertCount + vertCount - 1);
			tosame[1].push_back((iy + 0) * vertCount + vertCount - 2);
			tosame[1].push_back((iy + 1) * vertCount + vertCount - 2);

			tosame[1].push_back((iy + 1) * vertCount + vertCount - 1);
			tosame[1].push_back((iy + 0) * vertCount + vertCount - 1);
			tosame[1].push_back((iy + 1) * vertCount + vertCount - 2);
		}

	}

	// Upper and lower (to lower)
	for (size_t ix = 0; ix < vertCount - 2; ix+=2)
	{
		if (clockwise)
		{
			tolower[0].push_back(0 * vertCount + ix + 0);
			tolower[0].push_back(1 * vertCount + ix + 1);
			tolower[0].push_back(1 * vertCount + ix + 0);

			tolower[0].push_back(0 * vertCount + ix + 0);
			tolower[0].push_back(0 * vertCount + ix + 2);
			tolower[0].push_back(1 * vertCount + ix + 1);

			tolower[0].push_back(0 * vertCount + ix + 2);
			tolower[0].push_back(1 * vertCount + ix + 2);
			tolower[0].push_back(1 * vertCount + ix + 1);

			tolower[2].push_back((vertCount - 2) * vertCount + ix + 0);
			tolower[2].push_back((vertCount - 2) * vertCount + ix + 1);
			tolower[2].push_back((vertCount - 1) * vertCount + ix + 0);

			tolower[2].push_back((vertCount - 1) * vertCount + ix + 0);
			tolower[2].push_back((vertCount - 2) * vertCount + ix + 1);
			tolower[2].push_back((vertCount - 1) * vertCount + ix + 2);

			tolower[2].push_back((vertCount - 2) * vertCount + ix + 1);
			tolower[2].push_back((vertCount - 2) * vertCount + ix + 2);
			tolower[2].push_back((vertCount - 1) * vertCount + ix + 2);
		}
		else
		{
			tolower[0].push_back(1 * vertCount + ix + 1);
			tolower[0].push_back(0 * vertCount + ix + 0);
			tolower[0].push_back(1 * vertCount + ix + 0);

			tolower[0].push_back(0 * vertCount + ix + 2);
			tolower[0].push_back(0 * vertCount + ix + 0);
			tolower[0].push_back(1 * vertCount + ix + 1);

			tolower[0].push_back(1 * vertCount + ix + 2);
			tolower[0].push_back(0 * vertCount + ix + 2);
			tolower[0].push_back(1 * vertCount + ix + 1);

			tolower[2].push_back((vertCount - 2) * vertCount + ix + 1);
			tolower[2].push_back((vertCount - 2) * vertCount + ix + 0);
			tolower[2].push_back((vertCount - 1) * vertCount + ix + 0);

			tolower[2].push_back((vertCount - 2) * vertCount + ix + 1);
			tolower[2].push_back((vertCount - 1) * vertCount + ix + 0);
			tolower[2].push_back((vertCount - 1) * vertCount + ix + 2);

			tolower[2].push_back((vertCount - 2) * vertCount + ix + 2);
			tolower[2].push_back((vertCount - 2) * vertCount + ix + 1);
			tolower[2].push_back((vertCount - 1) * vertCount + ix + 2);
		}

	}

	// Left and Right (to lower)
	for (size_t iy = 0; iy < vertCount - 2; iy+=2)
	{
		if (clockwise)
		{
			tolower[3].push_back((iy + 0) * vertCount + 0);
			tolower[3].push_back((iy + 0) * vertCount + 1);
			tolower[3].push_back((iy + 1) * vertCount + 1);

			tolower[3].push_back((iy + 1) * vertCount + 1);
			tolower[3].push_back((iy + 2) * vertCount + 1);
			tolower[3].push_back((iy + 2) * vertCount + 0);

			tolower[3].push_back((iy + 0) * vertCount + 0);
			tolower[3].push_back((iy + 1) * vertCount + 1);
			tolower[3].push_back((iy + 2) * vertCount + 0);

			tolower[1].push_back((iy + 0) * vertCount + vertCount - 2);
			tolower[1].push_back((iy + 0) * vertCount + vertCount - 1);
			tolower[1].push_back((iy + 1) * vertCount + vertCount - 2);

			tolower[1].push_back((iy + 1) * vertCount + vertCount - 2);
			tolower[1].push_back((iy + 2) * vertCount + vertCount - 1);
			tolower[1].push_back((iy + 2) * vertCount + vertCount - 2);

			tolower[1].push_back((iy + 0) * vertCount + vertCount - 1);
			tolower[1].push_back((iy + 2) * vertCount + vertCount - 1);
			tolower[1].push_back((iy + 1) * vertCount + vertCount - 2);
		}
		else
		{
			tolower[3].push_back((iy + 0) * vertCount + 1);
			tolower[3].push_back((iy + 0) * vertCount + 0);
			tolower[3].push_back((iy + 1) * vertCount + 1);

			tolower[3].push_back((iy + 2) * vertCount + 1);
			tolower[3].push_back((iy + 1) * vertCount + 1);
			tolower[3].push_back((iy + 2) * vertCount + 0);

			tolower[3].push_back((iy + 1) * vertCount + 1);
			tolower[3].push_back((iy + 0) * vertCount + 0);
			tolower[3].push_back((iy + 2) * vertCount + 0);

			tolower[1].push_back((iy + 0) * vertCount + vertCount - 1);
			tolower[1].push_back((iy + 0) * vertCount + vertCount - 2);
			tolower[1].push_back((iy + 1) * vertCount + vertCount - 2);

			tolower[1].push_back((iy + 2) * vertCount + vertCount - 1);
			tolower[1].push_back((iy + 1) * vertCount + vertCount - 2);
			tolower[1].push_back((iy + 2) * vertCount + vertCount - 2);

			tolower[1].push_back((iy + 2) * vertCount + vertCount - 1);
			tolower[1].push_back((iy + 0) * vertCount + vertCount - 1);
			tolower[1].push_back((iy + 1) * vertCount + vertCount - 2);
		}
	}

	// Geenerate normals 
	for (size_t i = 0; i < all_indices.size(); i += 3)
	{
		generate_normal(i, all_indices, verts, FLOATS_PER_VERTEX, model_spheric);
	}

	/*for (size_t j = 0; j < 4; j++)
	{
		for (size_t i = 0; i < tosame[j].size(); i += 3)
		{
			generate_normal(i, tosame[j], verts, FLOATS_PER_VERTEX, model_spheric);
		}
	}*/

	vao = 0; vbo = 0; ebo = 0;
}


void PlanetTile::generate_normal(size_t i, std::vector<uint16_t>& indices, std::vector<float>& verts, size_t FLOATS_PER_VERTEX,
	glm::mat4 model_spheric)
{
	float* vert0 = &verts[indices[i + 0] * FLOATS_PER_VERTEX];
	float* vert1 = &verts[indices[i + 1] * FLOATS_PER_VERTEX];
	float* vert2 = &verts[indices[i + 2] * FLOATS_PER_VERTEX];

	glm::vec3 a = glm::vec3(vert0[0], vert0[1], vert0[2]);
	glm::vec3 b = glm::vec3(vert1[0], vert1[1], vert1[2]);
	glm::vec3 c = glm::vec3(vert2[0], vert2[1], vert2[2]);
	a = model_spheric * glm::vec4(a, 1.0f);
	b = model_spheric * glm::vec4(b, 1.0f);
	c = model_spheric * glm::vec4(c, 1.0f);

	glm::vec3 an = glm::vec3(vert0[3], vert0[4], vert0[5]);
	glm::vec3 bn = glm::vec3(vert1[3], vert1[4], vert1[5]);
	glm::vec3 cn = glm::vec3(vert2[3], vert2[4], vert2[5]);

	glm::vec3 face_normal = glm::triangleNormal(a, b, c);
	an = glm::normalize(an + face_normal);
	bn = glm::normalize(bn + face_normal);
	cn = glm::normalize(cn + face_normal);

	vert0[3] = an.x; vert0[4] = an.y; vert0[5] = an.z;
	vert1[3] = bn.x; vert1[4] = bn.y; vert1[5] = bn.z;
	vert2[3] = cn.x; vert2[4] = cn.y; vert2[5] = cn.z;
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

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts[0]) * verts.size(), verts.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);

	do_attributes();

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glGenVertexArrays(4, tolower_vao);
	glGenBuffers(4, tolower_ebo);
	glGenVertexArrays(4, tosame_vao);
	glGenBuffers(4, tosame_ebo);

	for (size_t i = 0; i < 4; i++)
	{
		glBindVertexArray(tolower_vao[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tolower_ebo[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tolower[i]) * tolower[i].size(), tolower[i].data(), GL_STATIC_DRAW);

		do_attributes();

		glBindVertexArray(tosame_vao[i]);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tosame_ebo[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tosame[i]) * tosame[i].size(), tosame[i].data(), GL_STATIC_DRAW);

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

