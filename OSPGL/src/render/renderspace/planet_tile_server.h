#pragma once
#include "planet_tile.h"

#include <unordered_map>
#include "../../orbital/planet.h"
#include "quad_tree_node.h"
#include "../../util/defines.h"
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/normal.hpp>
#include "../renderlow/mesh.h"
#include "../../util/math_util.h"
#include <vector>


#define TILE_WORKER_THREADS 2

struct PlanetTileWorker
{
	size_t id;
	std::thread* thread;
	bool run;
};


// Handles generation and caching of planetary tiles
class PlanetTileServer
{
private:


public:

	int being_worked_on;
	int not_generated;

	// Used for CPU efficiency of threads
	std::mutex condition_mtx;
	std::condition_variable condition_var;

	PlanetTileWorker worker_threads[TILE_WORKER_THREADS];

	// Used to stop the worker threads from iterating over tiles while it's modified
	std::mutex tiles_mtx;

	Planet* planet;

	std::unordered_map<PlanetTilePath, PlanetTile*, PlanetTilePathHasher> tiles;

	// May return lower quality parent if it's still being loaded, in that 
	// case all sides will be tosame
	PlanetTile* load(PlanetTilePath path, bool low_up, bool low_right, bool low_down, bool low_left, bool now = false);
	void unload(PlanetTilePath path, bool unload_now = false);

	bool has_tile_geometry(PlanetTilePath path);

	// Unloads all unused tiles
	void unload_unused();

	// Uploads all used tiles which are not uploaded
	void upload_used();

	// Unloads unused tiles, uploads not-uploaded used tiles and
	// checks worker threads
	void update();

	// Minimum depth at which tiles get unloaded, bigger tiles
	// will be loaded even if not used
	size_t minDepthToUnload = 3;

	// Vertices in the side of each tile, smallest tiles
	// simply get smallest heightmap samples
	// Small planets (or asteroids) need either more vertices
	// or to be kept at a decent subdivision level
	size_t verticesPerSide = 42 + 1;

	void rebuild_all();

	// Returns true if no tile is being worked on
	bool is_built();

	PlanetTileServer(Planet* planet);
	~PlanetTileServer();
};

