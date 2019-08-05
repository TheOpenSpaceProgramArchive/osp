#pragma once
#include <cstdint>

// Maximum depth, otherwise the algorithm
// would keep going until infinity
#define MAX_QUADTREE_DEPTH 8

class QuadTreeNode
{
public:
	
	enum QuadTreeSide
	{
		NORTH,
		EAST,
		SOUTH,
		WEST
	};

	enum QuadTreeCuadrant
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


	QuadTreeNode(QuadTreeNode* parent);
	~QuadTreeNode();
};

