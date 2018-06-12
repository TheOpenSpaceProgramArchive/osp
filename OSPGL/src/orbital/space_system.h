#pragma once

#include "space_body.h"

class SpaceSystem
{
public:

	std::vector<SpaceBody> bodies;

	SpaceSystem();
	~SpaceSystem();
};

