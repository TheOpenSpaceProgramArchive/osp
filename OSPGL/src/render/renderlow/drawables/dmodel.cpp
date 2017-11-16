#include "dmodel.h"



void dmodel::draw(glm::mat4 view, glm::mat4 proj)
{
	glm::mat4 tform_mat = tform.build_matrix();

	// Use shader
	sh->use();

	// Set uniforms
	sh->setmat4("model", tform_mat);
	sh->setmat4("view", view);
	sh->setmat4("proj", proj);

	// Draw
	glBindVertexArray(mh->vao);
	glDrawArrays(draw_type, 0, mh->vertices.size());
	
}

dmodel::dmodel(mesh* mh)
{
	dmodel(mh, NULL);
}


dmodel::dmodel(mesh* mh, shader* sh, uint draw_type)
{
	this->mh = mh;
	this->draw_type = draw_type;

	if (sh == NULL)
	{
		this->sh = g_shader;
	}
	else
	{
		this->sh = sh;
	}

}

dmodel::~dmodel()
{
}
