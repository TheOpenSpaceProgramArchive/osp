#include "output_spn.h"
#include "../surface_provider.h"

std::string OutputSPN::get_name()
{
	return "Output";
}

void OutputSPN::process(size_t length)
{

}

void OutputSPN::create(SurfaceProvider* surf)
{
	in_attribute[HEIGHT] = surf->create_attribute("Height", id, true, V1);
}
