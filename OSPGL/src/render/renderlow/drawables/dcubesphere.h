#pragma once
#include "../image.h"
#include "../shader.h"
#include "../transform.h"
#include "../drawable.h"
#include "../mesh.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>


struct CubeSphereNode
{
	glm::vec3 rel_pos;
	size_t level;

	// The less detailed parent
	CubeSphereNode* parent;
	// More detailed children
	CubeSphereNode *tl, *tr, *bl, *br;

	Mesh mesh;
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


public:

	float heightmap_power = 0.05f;

	// In this order: px,nx,py,ny,pz,nz ALWAYS
	std::vector<Image> cubemap;

	// Generates points from 0->1, as many in each side as detail gives
	// and they are transformed by tform TODO: Add bounds to img
	std::vector<CubeSpherePoint> make_cube_face(size_t detail, glm::mat4 tform, Image* img, size_t inv);
	// Bends points (from 0->1) from the origin (0, 0) to keep radius (1)
	void bend_cube_face(std::vector<CubeSpherePoint>* points, bool adv_mapping);

	CubeSphereNode rootx, rootmx, rooty, rootmy, rootz, rootmz;

	double radius;

	// The real-scaled position of the cubesphere
	glm::dvec3 world_pos;

	// Generates base LODs
	void generate_base();

	virtual void draw(glm::mat4 view, glm::mat4 proj) override;

	void generate_face(glm::vec3 trans, glm::vec3 rot, Mesh* target, Image* img, size_t inv);

	// Loads a cubemap given base path assuming the textures are named as expected (see top)
	void load_cubemap(std::string base_path);

	DCubeSphere();
	~DCubeSphere();
};

