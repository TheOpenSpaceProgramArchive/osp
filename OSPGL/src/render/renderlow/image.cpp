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

	for (size_t i = 0; i < width * height; i+=3)
	{
		Pixel n;
		n.r = data[i];
		n.g = data[i + 1];
		n.b = data[i + 2];
		n.a = data[i + 3];
		pixels.push_back(n);
	}
}

Image::Image()
{
}


Image::~Image()
{
}
