#pragma once
#include "../image.h"
#include "../shader.h"
#include "../transform.h"
#include "../drawable.h"
#include "../mesh.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/compatibility.hpp>

struct CubeSphereNode
{
	bool being_worked = false;

	bool dirty = false;

	enum Face
	{
		PX, NX, PY, NY, PZ, NZ
	};

	// In this order: (px, nx, py, ny, pz, nz)
	Face face;

	// The bounds of this node (0->1 on the cubesphere face)
	// (x,y) = (min_x,min_y) | (z,w) = (max_x,max_y)
	glm::vec4 bounds;
	size_t detail;

	// NULL if we don't have a subdivision
	CubeSphereNode* child;
	// NULL if we are a parent node
	CubeSphereNode* parent;

	Mesh mesh;

	// Calls this function on all children
	// Keep in mind it purely sends the OpenGL draw command
	// you need to setup-shaders outside
	void draw_recursive();

	// This function only needs to be called on root nodes
	void apply_viewpoint(glm::vec3 p, size_t min_detail = 32);

	// Will recursively generate higher and higher quality children
	// until enough detail is achieved
	void generate_children(glm::vec3 p, glm::vec3 surf_p);

	void collapse();

	CubeSphereNode* get_dirty();

	CubeSphereNode()
	{
		bounds = glm::vec4(0.0, 0.0, 1.0, 1.0);
		detail = 32;
	}
};

struct CubeSpherePoint
{
	glm::vec3 p;
	glm::vec3 n;
	float h;

	CubeSpherePoint(glm::vec3 np, float nh)
	{
		p = np; //< hmm
		h = nh;
	}
};

class DCubeSphere : public Drawable
{
private:
	// Used for softening LOD generation
	size_t update_count;

public:



	// Updated by the worker
	std::vector<CubeSphereNode*> require_upload;
	bool worker_run;
	std::thread worker;

	float heightmap_power = 0.05f;

	// In this order: px,nx,py,ny,pz,nz ALWAYS
	std::vector<Image> cubemap;

	void generate(CubeSphereNode* target);

	// Generates points from 0->1, as many in each side as detail gives
	// and they are transformed by tform TODO: Add bounds to img
	static std::vector<CubeSpherePoint> make_cube_face(size_t detail, glm::mat4 tform, Image* img, size_t inv, 
		glm::vec4 our_bounds, glm::vec4 child_bounds, float hpower);
	// Bends points (from 0->1) from the origin (0, 0) to keep radius (1)
	static void bend_cube_face(std::vector<CubeSpherePoint>* points, float adv_mult = 0.333f);

	void generate_face(glm::vec3 trans, glm::vec3 rot, Mesh* target, Image* img, size_t inv,
		glm::vec4 our_bounds, glm::vec4 child_bounds, bool flip_normal, size_t detail);

	CubeSphereNode* get_dirty();

	CubeSphereNode rootx, rootmx, rooty, rootmy, rootz, rootmz;

	double radius;

	// The real-scaled position of the cubesphere
	glm::dvec3 world_pos;

	// Generates base LODs
	void generate_base();

	glm::mat4 model;

	virtual void draw(glm::mat4 view, glm::mat4 proj) override;

	void launch_worker();

	// Needs to be called every frame, pools the worker to see
	// if anything requires OpenGL updates
	// Also updates the cube sphere nodes data as the camera moves
	// P is given in correcteed coordinates, aka, mapping the planet
	// to the normalized coordinates, and mapping that point too.
	// (Dividing the coordinate system by the planet's radius, and 
	// centering it on the planet)
	void update(glm::vec3 p);

	// Loads a cubemap given base path assuming the textures are named as expected (see top)
	void load_cubemap(std::string base_path);

	DCubeSphere();
	~DCubeSphere();
};

void cubesphere_worker(DCubeSphere* owner);

