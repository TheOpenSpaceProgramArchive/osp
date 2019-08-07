#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

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

	QuadTreeQuadrant quad;

	// Can be in another side of the cube, so be careful
	// North, East, South, West
	QuadTreeNode* neighbors[4];

	// Children
	// Northwest, Northeast, SoutWest, SouthEast
	QuadTreeNode* children[4];

	// Parent
	QuadTreeNode* parent;

	// Patch (Renderable mesh)


	// 0 is a root node
	size_t depth;

	// Returns true if split was possible
	bool split(bool get_neighbors = true);

	// Merges all children, destroying them
	bool merge();

	// A leaf node has no children
	bool has_children();

	// Recursively splits until we get a node with depth equal to maxDepth
	// It will automatically merge not-neccesary nodes, this is
	// the central function of the planet rendering system
	QuadTreeNode* get_recursive(glm::dvec2 coord, size_t maxDepth);

	glm::dvec2 get_center();

	// Returns a equivalent to QuadTreeQuadrant if inside, -1 if outside
	int get_quadrant(glm::dvec2 coord);

	// Gets given quadrant, splits if neccesary
	QuadTreeNode* get_or_split(QuadTreeQuadrant quad);

	void merge_all_but(QuadTreeNode* node);

	void obtain_neighbors(QuadTreeQuadrant quad);

	// Draws a "widget" of the subdivided quad tree nodes
	void draw_gui(int guiSize, glm::dvec2 focusPoint, QuadTreeNode* onNode);

	// Gets the path to this quad tree node, from its parent to the node
	// For example, a node may be {NW, NW, NE}, the first quadrant is the 
	// root, second is parent of the parent and last is the parent
	std::vector<QuadTreeQuadrant> getPath();

	// Gets all nodes with no children, sons of this node
	std::vector<QuadTreeNode*> getAllLeafNodes();

	QuadTreeNode();
	QuadTreeNode(QuadTreeNode* n_nbor, QuadTreeNode* e_nbor, QuadTreeNode* s_nbor, QuadTreeNode* w_nbor);
	QuadTreeNode(QuadTreeNode* parent, QuadTreeQuadrant quad);
	~QuadTreeNode();
};
