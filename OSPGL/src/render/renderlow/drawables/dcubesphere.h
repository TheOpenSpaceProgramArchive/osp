#pragma once
#include "../image.h"
#include "../shader.h"
#include "../transform.h"
#include "../drawable.h"
#include "../mesh.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>

struct CubeSphereMap
{

};

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

class DCubeSphere : public Drawable
{
private:


public:

	// Generates points from 0->1, as many in each side as detail gives
	// and they are transformed by tform
	std::vector<glm::vec3> make_cube_face(size_t detail, glm::mat4 tform);
	// Bends points (from 0->1) from the origin (0, 0) to keep radius (1)
	void bend_cube_face(std::vector<glm::vec3>* points);

	CubeSphereNode rootx, rootmx, rooty, rootmy, rootz, rootmz;

	double radius;

	// The real-scaled position of the cubesphere
	glm::dvec3 world_pos;

	// Generates base LODs
	void generate_base();

	virtual void draw(glm::mat4 view, glm::mat4 proj) override;

	void generate_face(glm::vec3 trans, glm::vec3 rot, Mesh* target);

	DCubeSphere();
	~DCubeSphere();
};

