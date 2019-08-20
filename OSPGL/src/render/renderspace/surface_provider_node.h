#pragma once
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <spdlog/spdlog.h>

class SurfaceProvider;

struct SurfaceProviderAttribute
{
	bool is_input;
	int id;
	std::string name;
	std::vector<float> values;
	int owner_id;

	// Do we override the values?
	bool has_values;

	// Both on OUTPUT and INPUT attributes
	// INPUTs should have only one link
	std::vector<int> links;

	void remove_link(int end_point)
	{
		int at = -1;
		for (int i = 0; i < links.size(); i++)
		{
			if (links[i] == end_point)
			{
				at = i;
			}
		}

		if (at >= 0)
		{
			links.erase(links.begin() + at);
		}
		else
		{
			spdlog::get("OSP")->error("Tried to remove a link, but could not find it!");
		}
	}
};

class SurfaceProviderNode
{
public:

	int id;

	std::unordered_map<int, SurfaceProviderAttribute*> in_attribute;
	std::unordered_map<int, SurfaceProviderAttribute*> out_attribute;

	virtual std::string get_name() = 0;

	// When this is called all attributes have valid data on them (from the inputs)
	// You should write to values in the output attributes
	// length is the ammount of values to write, even if you are a single constant
	virtual void process(size_t length) = 0;

	// Request attributes here
	virtual void create(SurfaceProvider* surf) = 0;

	// Draw the Imgui for a given attribute, feel free to do nothing
	// Return true if the terrain needs to update
	virtual bool do_imgui(int id) { return false; }

	// Obtains neccesary values from inputs, writes outputs and recursively propagates
	// down to the inputs, until all values are satisfied
	// Returns true if all was satisfied
	bool propagate(SurfaceProvider* surf, size_t length);
};