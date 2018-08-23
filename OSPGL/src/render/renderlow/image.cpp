#include "image.h"



Image Image::from_file(std::string path)
{
	Image out;
	out.load_from_file(path);
	return out;
}

void Image::load_from_file(std::string path)
{
	auto log = spd::get("OSP");

	int w, h, channels;
	uint8_t* data = stbi_load(path.c_str(), &w, &h, &channels, 4);

	if (data == NULL)
	{
		log->error("Could not load image! ({})", path);
	}

	width = w;
	height = h;

	pixels.reserve(width * height);

	for (size_t i = 0; i < width * height * 4; i+=4)
	{
		Pixel n;
		n.r = data[i];
		n.g = data[i + 1];
		n.b = data[i + 2];
		n.a = data[i + 3];
		pixels.push_back(n);
	}
}

Pixel Image::get_pixel(float x, float y)
{
	size_t rx = x * (width - 1);
	size_t ry = y * (height - 1);
	return get_pixel(rx, ry);
}

Pixel Image::get_pixel(size_t x, size_t y)
{
	assert(x < width);
	assert(y < height);
	size_t i = y * width + x;
	return pixels[i];
}

Image::Image()
{
}


Image::~Image()
{
}
