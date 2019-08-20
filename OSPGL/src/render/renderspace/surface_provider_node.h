#pragma once
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <spdlog/spdlog.h>

class SurfaceProvider;

enum ValueType
{
	V1,	// length * 1	floats
	V2,	// length * 2	floats
	V3,	// length * 3	floats
	ANY, // The node handles conversion to the other types

	END_MARKER,
};

struct SurfaceProviderAttribute
{


	bool is_input;
	int id;
	std::string name;
	ValueType val_type;
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

	static std::string valtype_to_str(ValueType val)
	{
		if (val == V1)
		{
			return "v1";
		}
		else if (val == V2)
		{
			return "v2";
		}
		else if (val == V3)
		{
			return "v3";
		}
		else
		{
			return "undef";
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


	// Utility functions
	ValueType pick_val_type(ValueType current, bool* set_dirty);
};