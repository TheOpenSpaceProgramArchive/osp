#include "quad_tree_node.h"



bool QuadTreeNode::split()
{
	// We cannot split if we have children or we are at max depth
	// TODO: Add a limit to how many splits per frame can happen
	if (!has_children() || depth >= MAX_QUADTREE_DEPTH)
	{
		return false;
	}

	// Create new children
	QuadTreeNode* nw = new QuadTreeNode(this);
	QuadTreeNode* ne = new QuadTreeNode(this);
	QuadTreeNode* sw = new QuadTreeNode(this);
	QuadTreeNode* se = new QuadTreeNode(this);

	// Assign them
	children[NORTH_WEST] = nw;
	children[NORTH_EAST] = ne;
	children[SOUTH_WEST] = sw;
	children[SOUTH_EAST] = se;

	return true;
}

bool QuadTreeNode::merge()
{
	if (!has_children())
	{
		return false;
	}
	else
	{
		delete children[0]; children[0] = NULL;
		delete children[1]; children[1] = NULL;
		delete children[2]; children[2] = NULL;
		delete children[3]; children[3] = NULL;
	}
}

bool QuadTreeNode::has_children()
{
	// If we have any children then it's already
	// a not-leaf, so just checking the first one works
	return children[0] != NULL;
}


QuadTreeNode::QuadTreeNode(QuadTreeNode* p) : parent(p)
{
	children[0] = NULL; children[1] = NULL; children[2] = NULL; children[3] = NULL;
}


QuadTreeNode::~QuadTreeNode()
{
	merge();
}
