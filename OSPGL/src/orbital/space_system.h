#pragma once

#include "space_body.h"

class space_system
{
public:

	std::vector<space_body> bodies;

	space_system();
	~space_system();
};

