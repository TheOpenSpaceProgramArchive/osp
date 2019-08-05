#include "render_body.h"



void RenderBody::generate_mesh_detailed(glm::dvec3 cameraPos, double dist)
{

}

void RenderBody::generate_mesh_lowdetail(glm::dvec3 cameraPos, double dist)
{
	// TODO: Implement
}

void RenderBody::generate_mesh_billboard(glm::dvec3 cameraPos, double dist)
{
	// TODO: Implement
}

void RenderBody::generate_mesh(glm::dvec3 camPos)
{
	double dist = camPos.length();

	if (dist <= detail_dist)
	{
		generate_mesh_detailed(camPos, dist);
	}
	else if (dist <= far_dist)
	{
		generate_mesh_lowdetail(camPos, dist);
	}
	else
	{
		generate_mesh_billboard(camPos, dist);
	}
}

RenderBody::RenderBody()
{
}


RenderBody::~RenderBody()
{
}
