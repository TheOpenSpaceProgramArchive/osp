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

		if (attr->val_type != input.second->val_type && input.second->val_type != ANY)
		{
			// Type mismatch
			return false;
		}

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

int SurfaceProviderNode::find_attribute(SurfaceProviderAttribute * attr)
{
	for (auto inlet : in_attribute)
	{
		if (inlet.second == attr)
		{
			return inlet.first;
		}
	}
	
	for (auto outlet : out_attribute)
	{
		if (outlet.second == attr)
		{
			return outlet.first;
		}
	}

	return -1;
}

SurfaceProviderAttribute * SurfaceProviderNode::find_attribute_by_id(int id)
{
	auto in = in_attribute.find(id);
	
	if (in != in_attribute.end())
	{
		return in->second;
	}

	auto out = out_attribute.find(id);

	if (out != out_attribute.end())
	{
		return out->second;
	}

	return NULL;
}

ValueType SurfaceProviderNode::pick_val_type(ValueType current, bool * set_dirty)
{
	ValueType selected = current;

	std::string current_str = SurfaceProviderAttribute::valtype_to_str(current);
	if (ImGui::BeginCombo("#", current_str.c_str()))
	{
		for (int i = 0; i < ValueType::ANY; i++)
		{
			std::string label = SurfaceProviderAttribute::valtype_to_str(ValueType(i));
			if (ImGui::Selectable(label.c_str()))
			{
				selected = ValueType(i);
			}
		}
		ImGui::EndCombo();
	}

	*set_dirty |= selected != current;

	return selected;
}