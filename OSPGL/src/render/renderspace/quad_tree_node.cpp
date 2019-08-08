#include "quad_tree_node.h"
#include <imgui/imgui.h>


bool QuadTreeNode::split(bool get_neighbors)
{
	// We cannot split if we have children or we are at max depth
	// TODO: Add a limit to how many splits per frame can happen
	if (has_children())
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

	if (get_neighbors)
	{
		nw->obtain_neighbors(NORTH_WEST);
		ne->obtain_neighbors(NORTH_EAST);
		sw->obtain_neighbors(SOUTH_WEST);
		se->obtain_neighbors(SOUTH_EAST);
	}

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
	if (depth >= 1)
	{
		// Obtain all neighbors to avoid diagonal loss of detail
		obtain_neighbors(quad);
		neighbors[NORTH]->obtain_neighbors(neighbors[NORTH]->quad);
		neighbors[EAST]->obtain_neighbors(neighbors[EAST]->quad);
		neighbors[SOUTH]->obtain_neighbors(neighbors[SOUTH]->quad);
		neighbors[WEST]->obtain_neighbors(neighbors[WEST]->quad);
	}

	if (depth < maxDepth)
	{
		int result = get_quadrant(coord);
		if (result == -1)
		{
			return NULL; //< Should never happen
		}

		QuadTreeQuadrant quad = (QuadTreeQuadrant)result;
		QuadTreeNode* child = get_or_split(quad);
		child->obtain_neighbors(quad);

		// We can merge every other node that is not child
		merge_all_but(child);

		return child->get_recursive(coord, maxDepth);

	}
	else
	{
		return this;
	}
}

glm::dvec2 QuadTreeNode::get_center()
{
	return glm::dvec2(min_point.x + size / 2.0, min_point.y + size / 2.0);
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
		split(false);
	}

	return children[quad];
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

void QuadTreeNode::obtain_neighbors(QuadTreeQuadrant quad)
{
	// Obtain neighbors
	if (quad == NORTH_WEST)
	{
		neighbors[EAST] = parent->children[NORTH_EAST];
		neighbors[SOUTH] = parent->children[SOUTH_WEST];

		if (parent->neighbors[WEST]->planetside != planetside)
		{
			if (planetside == NX || planetside == PZ)
			{
				neighbors[WEST] = parent->neighbors[WEST]->get_or_split(SOUTH_EAST);
			}
			else if (planetside == NY)
			{
				neighbors[WEST] = parent->neighbors[WEST]->get_or_split(SOUTH_WEST);
			}
			else
			{
				neighbors[WEST] = parent->neighbors[WEST]->get_or_split(NORTH_EAST);
			}
		}
		else
		{
			neighbors[WEST] = parent->neighbors[WEST]->get_or_split(NORTH_EAST);
		}
			
		if (parent->neighbors[NORTH]->planetside != planetside)
		{
			if (planetside == PX || planetside == NY)
			{
				neighbors[NORTH] = parent->neighbors[NORTH]->get_or_split(SOUTH_EAST);
			}
			else if (planetside == PZ || planetside == PY)
			{
				neighbors[NORTH] = parent->neighbors[NORTH]->get_or_split(NORTH_EAST);
			}
			else
			{
				neighbors[NORTH] = parent->neighbors[NORTH]->get_or_split(SOUTH_WEST);
			}
		}
		else
		{
			neighbors[NORTH] = parent->neighbors[NORTH]->get_or_split(SOUTH_WEST);
		}
	}
	else if (quad == NORTH_EAST)
	{
		neighbors[WEST] = parent->children[NORTH_WEST];
		neighbors[SOUTH] = parent->children[SOUTH_EAST];

		if(parent->neighbors[EAST]->planetside != planetside)
		{
			if (planetside == NZ || planetside == NX)
			{
				neighbors[EAST] = parent->neighbors[EAST]->get_or_split(SOUTH_WEST);
			}
			else if (planetside == NY)
			{
				neighbors[EAST] = parent->neighbors[EAST]->get_or_split(SOUTH_EAST);
			}
			else
			{
				neighbors[EAST] = parent->neighbors[EAST]->get_or_split(NORTH_WEST);
			}
		}
		else
		{
			neighbors[EAST] = parent->neighbors[EAST]->get_or_split(NORTH_WEST);
		}
		

		if (parent->neighbors[NORTH]->planetside != planetside)
		{
			if (planetside == NZ)
			{
				neighbors[NORTH] = parent->neighbors[NORTH]->get_or_split(NORTH_WEST);
			}
			else if (planetside == NX)
			{
				neighbors[NORTH] = parent->neighbors[NORTH]->get_or_split(SOUTH_EAST);
			}
			else if (planetside == PX || planetside == NY)
			{
				neighbors[NORTH] = parent->neighbors[NORTH]->get_or_split(SOUTH_WEST);
			}
			else
			{
				neighbors[NORTH] = parent->neighbors[NORTH]->get_or_split(SOUTH_EAST);
			}
		}
		else
		{
			neighbors[NORTH] = parent->neighbors[NORTH]->get_or_split(SOUTH_EAST);
		}
	}
	else if (quad == SOUTH_WEST)
	{
		neighbors[NORTH] = parent->children[NORTH_WEST];
		neighbors[EAST] = parent->children[SOUTH_EAST];

		if (parent->neighbors[WEST]->planetside != planetside)
		{
			if (planetside == NX || planetside == PZ)
			{
				neighbors[WEST] = parent->neighbors[WEST]->get_or_split(NORTH_EAST);
			}
			else if (planetside == PY)
			{
				neighbors[WEST] = parent->neighbors[WEST]->get_or_split(NORTH_WEST);
			}
			else
			{
				neighbors[WEST] = parent->neighbors[WEST]->get_or_split(SOUTH_EAST);
			}
		}
		else
		{
			neighbors[WEST] = parent->neighbors[WEST]->get_or_split(SOUTH_EAST);
		}

		if (parent->neighbors[SOUTH]->planetside != planetside)
		{
			if (planetside == PX || planetside == PY)
			{
				neighbors[SOUTH] = parent->neighbors[SOUTH]->get_or_split(NORTH_EAST);
			}
			else if (planetside == PZ)
			{
				neighbors[SOUTH] = parent->neighbors[SOUTH]->get_or_split(SOUTH_EAST);
			}
			else
			{
				neighbors[SOUTH] = parent->neighbors[SOUTH]->get_or_split(NORTH_WEST);
			}
		}
		else
		{
			neighbors[SOUTH] = parent->neighbors[SOUTH]->get_or_split(NORTH_WEST);
		}

	}
	else if (quad == SOUTH_EAST)
	{
		neighbors[NORTH] = parent->children[NORTH_EAST];
		neighbors[WEST] = parent->children[SOUTH_WEST];

		if (parent->neighbors[EAST]->planetside != planetside)
		{
			if (planetside == NZ || planetside == NX)
			{
				neighbors[EAST] = parent->neighbors[EAST]->get_or_split(NORTH_WEST);
			}
			else if (planetside == PY)
			{
				neighbors[EAST] = parent->neighbors[EAST]->get_or_split(NORTH_EAST);
			}
			else
			{
				neighbors[EAST] = parent->neighbors[EAST]->get_or_split(SOUTH_WEST);
			}
		}
		else
		{
			neighbors[EAST] = parent->neighbors[EAST]->get_or_split(SOUTH_WEST);
		}


		if (parent->neighbors[SOUTH]->planetside != planetside)
		{
			if (planetside == PX || planetside == PY)
			{
				neighbors[SOUTH] = parent->neighbors[SOUTH]->get_or_split(NORTH_WEST);
			}
			else if (planetside == NZ)
			{
				neighbors[SOUTH] = parent->neighbors[SOUTH]->get_or_split(SOUTH_WEST);
			}
			else
			{
				neighbors[SOUTH] = parent->neighbors[SOUTH]->get_or_split(NORTH_EAST);
			}
		}
		else
		{
			neighbors[SOUTH] = parent->neighbors[WEST]->get_or_split(NORTH_EAST);
		}

	}
}




std::vector<QuadTreeNode::QuadTreeQuadrant> QuadTreeNode::getPath()
{

	if (depth > 0)
	{

		std::vector<QuadTreeQuadrant> fromParent = parent->getPath();

		if (parent->children[NORTH_WEST] == this)
		{
			fromParent.push_back(NORTH_WEST);
		}

		if (parent->children[NORTH_EAST] == this)
		{
			fromParent.push_back(NORTH_EAST);
		}

		if (parent->children[SOUTH_WEST] == this)
		{
			fromParent.push_back(SOUTH_WEST);
		}

		if (parent->children[SOUTH_EAST] == this)
		{
			fromParent.push_back(SOUTH_EAST);
		}

		return fromParent;
	}
	else
	{
		return std::vector<QuadTreeQuadrant>();
	}
	
}


std::vector<QuadTreeNode*> QuadTreeNode::getAllLeafNodes()
{
	std::vector<QuadTreeNode*> out;

	for (size_t i = 0; i < 4; i++)
	{
		if (children[i] != NULL)
		{
			if (children[i]->has_children())
			{
				std::vector<QuadTreeNode*> fromChild = children[i]->getAllLeafNodes();
				out.insert(out.end(), fromChild.begin(), fromChild.end());
			}
			else
			{
				out.push_back(children[i]);
			}
		}
		else
		{
			out.push_back(this);
		}
	}

	return out;
}

QuadTreeNode::QuadTreeNode()
{
	depth = 0;
	min_point = glm::dvec2(0.0, 0.0);
	size = 1.0;
}

QuadTreeNode::QuadTreeNode(QuadTreeNode* n_nbor, QuadTreeNode* e_nbor, QuadTreeNode* s_nbor, QuadTreeNode* w_nbor) : parent(NULL)
{
	depth = 0;
	min_point = glm::dvec2(0.0, 0.0);
	size = 1.0;

	neighbors[NORTH] = n_nbor;
	neighbors[EAST] = e_nbor;
	neighbors[SOUTH] = s_nbor;
	neighbors[WEST] = w_nbor;

	n_nbor->neighbors[SOUTH] = this;
	e_nbor->neighbors[WEST] = this;
	s_nbor->neighbors[NORTH] = this;
	w_nbor->neighbors[EAST] = this;
}

QuadTreeNode::QuadTreeNode(QuadTreeNode* p, QuadTreeQuadrant quad) : parent(p)
{
	this->quad = quad;
	this->planetside = p->planetside;

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

void QuadTreeNode::draw_gui(int SCL, glm::dvec2 focusPoint, QuadTreeNode* onNode)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 min = ImGui::GetItemRectMin();

	ImVec2 tl = ImVec2(min.x + min_point.x * SCL, min.y + min_point.y * SCL);
	ImVec2 tr = ImVec2(tl.x + size * SCL, tl.y);
	ImVec2 bl = ImVec2(tl.x, tl.y + size * SCL);
	ImVec2 br = ImVec2(tl.x + size * SCL, tl.y + size * SCL);

	drawList->AddLine(tl, tr, ImColor(1.0f, 1.0f, 1.0f), 1.0f);
	drawList->AddLine(tr, br, ImColor(1.0f, 1.0f, 1.0f), 1.0f);
	drawList->AddLine(br, bl, ImColor(1.0f, 1.0f, 1.0f), 1.0f);
	drawList->AddLine(bl, tl, ImColor(1.0f, 1.0f, 1.0f), 1.0f);

	if (has_children())
	{
		children[0]->draw_gui(SCL, focusPoint, onNode);
		children[1]->draw_gui(SCL, focusPoint, onNode);
		children[2]->draw_gui(SCL, focusPoint, onNode);
		children[3]->draw_gui(SCL, focusPoint, onNode);
	}

	if (onNode == this && depth == 0)
	{
		ImVec2 target = ImVec2(focusPoint.x * SCL + min.x, focusPoint.y * SCL + min.y);
		drawList->AddCircle(target, 2.0f, ImColor(1.0f, 0.0f, 0.0f), 12, 1.0f);
	}
}

QuadTreeNode::QuadTreeSide QuadTreeNode::findTouchingSide(QuadTreeNode* node)
{
	if (neighbors[NORTH] == node)
	{
		return NORTH;
	}
	else if (neighbors[EAST] == node)
	{
		return EAST;
	}
	else if (neighbors[SOUTH] == node)
	{
		return SOUTH;
	}
	else
	{
		return WEST;
	}
}

bool QuadTreeNode::touches_any_edge()
{
	if (depth == 0)
	{
		return true;
	}

	for (size_t i = 0; i < 4; i++)
	{
		if (neighbors[i] != NULL && neighbors[i]->planetside != planetside)
		{
			return true;
		}
	}
}
