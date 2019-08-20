#include "input_spn.h"
#include "../surface_provider.h"

std::string InputSPN::get_name()
{
	return "Input";
}

void InputSPN::process(size_t length)
{

}

void InputSPN::create(SurfaceProvider* surf)
{
	out_attribute[SPHERE_POS] = surf->create_attribute("Sphere", id, false, V3);

	out_attribute[RADIUS] = surf->create_attribute("Planet Radius", id, false, V1);
}


