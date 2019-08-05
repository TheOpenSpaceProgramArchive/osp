#pragma once
#include <cstdint>
#include <glm/glm.hpp>

// Maximum depth, otherwise the algorithm
// would keep going until infinity
#define MAX_QUADTREE_DEPTH 8

class QuadTreeNode
{
private:


	
public:
	
	// Top-left bound of the node
	glm::dvec2 min_point;
	double size;

	enum QuadTreeSide
	{
		NORTH,
		EAST,
		SOUTH,
		WEST
	};

	enum QuadTreeQuadrant
	{
		NORTH_WEST,
		NORTH_EAST,
		SOUTH_WEST,
		SOUTH_EAST
	};

	// Children
	// Northwest, Northeast, Soutwest, SouthEast
	QuadTreeNode* children[4];

	// Parent
	QuadTreeNode* parent;

	// Patch (Renderable mesh)


	// 0 is a root node
	size_t depth;

	// Returns true if split was possible
	bool split();

	// Merges all children, destroying them
	bool merge();

	// A leaf node has no children
	bool has_children();

	// Recursively splits until we get a node with depth equal to maxDepth
	// It will automatically merge not-neccesary nodes, this is
	// the central function of the planet rendering system
	QuadTreeNode* get_recursive(glm::dvec2 coord, size_t maxDepth);

	// Returns a equivalent to QuadTreeQuadrant if inside, -1 if outside
	int get_quadrant(glm::dvec2 coord);

	// Gets given quadrant, splits if neccesary
	QuadTreeNode* get_or_split(QuadTreeQuadrant quad);

	void merge_all_but(QuadTreeNode* node);

	QuadTreeNode();
	QuadTreeNode(QuadTreeNode* parent, QuadTreeQuadrant quad);
	~QuadTreeNode();
};

