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
	QuadTreeNode* nw = new QuadTreeNode(this, NORTH_WEST);
	QuadTreeNode* ne = new QuadTreeNode(this, NORTH_EAST);
	QuadTreeNode* sw = new QuadTreeNode(this, SOUTH_WEST);
	QuadTreeNode* se = new QuadTreeNode(this, SOUTH_EAST);

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

QuadTreeNode* QuadTreeNode::get_recursive(glm::dvec2 coord, size_t maxDepth)
{
	if (depth < maxDepth)
	{
		int result = get_quadrant(coord);
		if (result == -1)
		{
			return NULL; //< Should never happen
		}

		QuadTreeQuadrant quad = (QuadTreeQuadrant)result;
		QuadTreeNode* child = get_or_split(quad);

		// We can merge every other node that is not child
		merge_all_but(child);

		return child->get_recursive(coord, maxDepth);
	}
	else
	{
		return this;
	}
}

int QuadTreeNode::get_quadrant(glm::dvec2 coord)
{
	// We expect a point inside the quadtree, so no safety checks

	if (coord.x >= min_point.x && coord.y >= min_point.y && coord.x < min_point.x + size / 2.0 && coord.y < min_point.y + size / 2.0)
	{
		return QuadTreeQuadrant::NORTH_WEST;
	}
	else if (coord.x >= min_point.x + size / 2.0 && coord.y >= min_point.y && coord.x < min_point.x + size && coord.y < min_point.y + size / 2.0)
	{
		return QuadTreeQuadrant::NORTH_EAST;
	}
	else if (coord.x >= min_point.x && coord.y >= min_point.y + size / 2.0 && coord.x < min_point.x + size / 2.0 && coord.y < min_point.y + size)
	{
		return QuadTreeQuadrant::SOUTH_WEST;
	}
	else if(coord.x >= min_point.x + size / 2.0 && coord.y >= min_point.y + size / 2.0 && coord.x < min_point.x + size && coord.y < min_point.y + size)
	{
		return QuadTreeQuadrant::SOUTH_EAST;
	}
	else
	{
		return -1;
	}
}

QuadTreeNode* QuadTreeNode::get_or_split(QuadTreeQuadrant quad)
{
	if (!has_children())
	{
		split();
	}

	if (quad == NORTH_WEST)
	{
		return children[0];
	}
	else if (quad == NORTH_EAST)
	{
		return children[1];
	}
	else if (quad == SOUTH_WEST)
	{
		return children[2];
	}
	else if (quad == SOUTH_EAST)
	{
		return children[3];
	}
	else
	{
		return NULL; //< Should never happen
	}
}

void QuadTreeNode::merge_all_but(QuadTreeNode* node)
{
	if (children[0] != node)
	{
		children[0]->merge();
	}

	if (children[1] != node)
	{
		children[1]->merge();
	}

	if (children[2] != node)
	{
		children[2]->merge();
	}

	if (children[3] != node)
	{
		children[3]->merge();
	}
}

QuadTreeNode::QuadTreeNode() : parent(NULL)
{
	depth = 0;
	min_point = glm::dvec2(0.0, 0.0);
	size = 1.0;
}

QuadTreeNode::QuadTreeNode(QuadTreeNode* p, QuadTreeQuadrant quad) : parent(p)
{
	children[0] = NULL; children[1] = NULL; children[2] = NULL; children[3] = NULL;

	depth = p->depth + 1;

	if (quad == NORTH_WEST)
	{
		min_point = parent->min_point;
	}
	else if (quad == NORTH_EAST)
	{
		min_point = glm::dvec2(parent->min_point.x + parent->size / 2.0, parent->min_point.y);
	}
	else if (quad == SOUTH_WEST)
	{
		min_point = glm::dvec2(parent->min_point.x, parent->min_point.y + parent->size / 2.0);
	}
	else if (quad == SOUTH_EAST)
	{
		min_point = glm::dvec2(parent->min_point.x + parent->size / 2.0, parent->min_point.y + parent->size / 2.0);
	}

	size = parent->size / 2.0;
	
}


QuadTreeNode::~QuadTreeNode()
{
	merge();
}
