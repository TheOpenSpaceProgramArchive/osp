#include "planet_tile_server.h"



PlanetTile* PlanetTileServer::load(PlanetTilePath path)
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

	if (!out->isUploaded())
	{
		out->upload(false, false, false, false);
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

glm::dvec2 PlanetTilePath::getMin()
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



PlanetTile::PlanetTile(PlanetTilePath nPath, size_t vertCount, const Planet& planet) : path(nPath.path, nPath.side)
{
	this->users = 0;
	this->verts = std::vector<float>();
	this->indices = std::vector<uint16_t>();

	// We use only position, normal and texture
	const int FLOATS_PER_VERTEX = 8;

	verts.resize(vertCount * vertCount * FLOATS_PER_VERTEX);

	// Generate all vertices
	for (size_t iy = 0; iy < vertCount; iy++)
	{	
		for (size_t ix = 0; ix < vertCount; ix++)
		{
			float x = (float)ix / ((float)vertCount - 1);
			float y = (float)iy / ((float)vertCount - 1);
			float height = 0.0f;

			Vertex out;

			out.pos = glm::vec3(x, y, height);

			verts[(iy * vertCount + ix) * FLOATS_PER_VERTEX + 0] = out.pos.x;
			verts[(iy * vertCount + ix) * FLOATS_PER_VERTEX + 1] = out.pos.y;
			verts[(iy * vertCount + ix) * FLOATS_PER_VERTEX + 2] = out.pos.z;
			verts[(iy * vertCount + ix) * FLOATS_PER_VERTEX + 3] = out.nrm.x;
			verts[(iy * vertCount + ix) * FLOATS_PER_VERTEX + 4] = out.nrm.y;
			verts[(iy * vertCount + ix) * FLOATS_PER_VERTEX + 5] = out.nrm.z;
			verts[(iy * vertCount + ix) * FLOATS_PER_VERTEX + 6] = out.uv.x;
			verts[(iy * vertCount + ix) * FLOATS_PER_VERTEX + 7] = out.uv.y;
		}
	}


	// Generate edge to lower quality vertices (TODO)

	// Generate indices for normal mesh
	for (size_t iy = 0; iy < vertCount - 1; iy++)
	{
		for (size_t ix = 0; ix < vertCount; ix++)
		{
			if (ix < vertCount - 1)
			{
				// Center
				indices.push_back(iy * vertCount + ix);
				// Down
				indices.push_back((iy + 1) * vertCount + ix);
				// Right
				indices.push_back(iy * vertCount + (ix + 1));
			}

			if (ix > 0)
			{
				// Center
				indices.push_back(iy * vertCount + ix);
				// Down
				indices.push_back((iy + 1) * vertCount + ix);
				// Down Left
				indices.push_back((iy + 1) * vertCount + (ix - 1));
			}
		}
	}

	// Generate indices for edge to lower quality meshes (TODO)

	vao = 0; vbo = 0; ebo = 0;
}

bool PlanetTile::isUploaded()
{
	// Just checking one is enough
	return vao != 0;
}

void PlanetTile::upload(bool u_lowq, bool r_lowq, bool d_lowq, bool l_downq)
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

	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// tex
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
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

	vao = 0;
	vbo = 0;
	ebo = 0;
}
