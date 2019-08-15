#include "planet_tile_server.h"
#include "surface_provider.h"


PlanetTile* PlanetTileServer::load(PlanetTilePath path, bool low_up, bool low_right, bool low_down, bool low_left, bool now)
{
	// We want no thread to mess with us here, 
	// lock for the duration of the whole function
	std::lock_guard<std::mutex> guard(tiles_mtx);

	auto it = tiles.find(path);
	PlanetTile* out;

	if (it != tiles.end())
	{
		out = it->second;
	}
	else
	{
		if (path.get_depth() == 0)
		{
			PlanetTile* nTile = new PlanetTile(path, verticesPerSide, *planet, true);
			nTile->is_generated = true;
			nTile->is_being_generated = false;
			tiles[path] = nTile;
			out = nTile;
		}
		else
		{
			// Create new tile, but don't generate for now
			PlanetTile* nTile = new PlanetTile(path, verticesPerSide, *planet);
			tiles[path] = nTile;
			out = nTile;
		}
	}

	if (out->is_generated)
	{
		out->used = true;
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
	}
	else
	{
		// We return NULL for now, or load it instantly if parent is not available
		PlanetTilePath parent = path; parent.path.pop_back();
		auto it = tiles.find(parent);
		if (it != tiles.end() && it->second->is_generated)
		{
			it->second->used = true;
			condition_var.notify_one();
			return NULL;
		}
		else
		{
			out->used = true;
			out->is_generated = true;
			out->is_being_generated = false;
			out->generate();
			return out;
		}
	}


	return out;
}

void PlanetTileServer::unload(PlanetTilePath path, bool unload_now)
{
	auto it = tiles.find(path);
	if (it != tiles.end())
	{
		it->second->used = false;

		if (unload_now)
		{
			it->second->unload();

			if (it->second->path.get_depth() >= minDepthToUnload)
			{
				delete it->second;
				tiles_mtx.lock();
				tiles.erase(it);
				tiles_mtx.unlock();
			}
		}
	}
}

void PlanetTileServer::unload_unused()
{
	std::vector<PlanetTilePath> toDelete;

	for (auto const& pair : tiles)
	{
		if (pair.second->used == false && pair.second->is_generated)
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
		if (pair.second->used && pair.second->is_generated)
		{
			if (!pair.second->isUploaded())
			{
				pair.second->upload();
				i++;
			}
		}
	}
}

void PlanetTileServer::update()
{
	any_being_worked = false;

	for (auto tile : tiles)
	{
		if (!tile.second->is_generated)
		{
			any_being_worked = true;
		}
	}

	unload_unused();
	upload_used();
}

void PlanetTileServer::rebuild_all()
{
	for (auto tile : tiles)
	{
		bool was_uploaded = false;

		if (tile.second->isUploaded())
		{
			tile.second->unload();
			was_uploaded = true;
		}
		
		tile.second->generate();

		if (was_uploaded)
		{
			tile.second->upload();
		}
	}
}

bool PlanetTileServer::is_built()
{
	return !any_being_worked;
}

void tile_worker_thread_function(PlanetTileServer* server, PlanetTileWorker* we)
{
	while (we->run)
	{
		std::unique_lock<std::mutex> lock(server->condition_mtx);
		server->condition_var.wait(lock);
		// Work needs to be done, we have awoken!

		PlanetTile* target = NULL;

		server->tiles_mtx.lock();
		for (auto tile : server->tiles)
		{
			if (tile.second->is_generated == false && !tile.second->is_being_generated)
			{
				tile.second->is_being_generated = true;
				target = tile.second;
				break;
			}
		}

		server->tiles_mtx.unlock();

		if (target != NULL)
		{
			target->generate();
			target->is_generated = true;
			target->is_being_generated = false;
		}
	}
}

PlanetTileServer::PlanetTileServer(Planet* planet)
{
	this->planet = planet;

	for (size_t i = 0; i < TILE_WORKER_THREADS; i++)
	{
		worker_threads[i].id = i;
		worker_threads[i].run = true;
		worker_threads[i].thread = new std::thread(tile_worker_thread_function, this, &worker_threads[i]);
	}
}


PlanetTileServer::~PlanetTileServer()
{
	for (size_t i = 0; i < TILE_WORKER_THREADS; i++)
	{
		condition_var.notify_all();
		worker_threads[i].run = false;
		worker_threads[i].thread->join();
		delete worker_threads[i].thread;
	}
}

