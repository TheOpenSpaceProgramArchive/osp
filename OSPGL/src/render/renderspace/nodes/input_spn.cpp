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
	out_attribute[SPHERE_X] = surf->create_attribute("Sphere X", id, false);
	out_attribute[SPHERE_Y] = surf->create_attribute("Sphere Y", id, false);
	out_attribute[SPHERE_Z] = surf->create_attribute("Sphere Z", id, false);

	out_attribute[RADIUS] = surf->create_attribute("Planet Radius", id, false);
}


