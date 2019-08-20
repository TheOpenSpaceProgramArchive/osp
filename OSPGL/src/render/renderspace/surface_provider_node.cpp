#include "surface_provider_node.h"
#include "surface_provider.h"

bool SurfaceProviderNode::propagate(SurfaceProvider* surf, size_t length)
{
	// Satisfy all inputs
	for (auto input : in_attribute)
	{

		if (input.second->has_values)
		{
			continue;
		}

		if (input.second->links.size() == 0)
		{
			return false;
		}

		SurfaceProviderAttribute* attr = surf->attributes[input.second->links[0]];
		SurfaceProviderNode* owner = surf->nodes[attr->owner_id];

		bool ret = owner->propagate(surf, length);
		if (ret == false)
		{
			return false;
		}

		input.second->values = attr->values;
	}

	// We now have every input, calculate outputs
	process(length);

	return true;
}
