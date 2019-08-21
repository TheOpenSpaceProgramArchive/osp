#pragma once
#include <vector>
#include <string>

#include <stb/stb_image.h>
#include "../../util/defines.h"


struct Pixel
{
	uint8_t r, g, b, a;
};

class Image
{
public:

	size_t width, height;
	std::vector<Pixel> pixels;

	static Image from_file(std::string path);
	void load_from_file(std::string path);

	// Normalized coordinates
	Pixel get_pixel(float x, float y);
	// Real coordinates
	Pixel get_pixel(size_t x, size_t y);

	void set_pixel(size_t x, size_t y, Pixel color);

	Image();
	~Image();
};

