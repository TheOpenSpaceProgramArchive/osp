#pragma once

#include <spdlog/spdlog.h>
#include "transform.h"

/*
	Base class for stuff which can be drawn
	by the low level renderer.

	Used as a base for models, lines, etc...

	Implements nothing but warners about the wrong
	method being called.

	All drawables are prefixed with 'd'

*/

class Drawable
{
public:


	virtual void draw(glm::mat4 view, glm::mat4 proj)
	{
		spdlog::get("OSP")->warn("Drawable base 'draw' called! This is not meant to happen.");
	}

	Drawable(){}

	~Drawable() {}
};

