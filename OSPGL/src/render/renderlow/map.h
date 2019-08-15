#pragma once
#include <string>
#include <vector>

// A map is similar to an image, but allows floating point data
// and projection. It can store both heightmaps (in floats) and
// color data (RGB)
// Maps use tiff data, and hence are different from Images
class Map
{
public:

	enum MapProjection
	{
		CYLINDRICAL,
	};

	std::vector<float> height_data;
	size_t height_width, height_height;

	MapProjection proj;

	// Coordinates are in radians
	float get_height(float lat, float lon, bool interpolate = true);

	// Loads all appropiate files from data_path
	Map(const std::string& data_path);
	~Map();
};

