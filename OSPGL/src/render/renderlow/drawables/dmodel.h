#pragma once

#include "../drawable.h"
#include "../shader.h"


/*
	A dmodel draws given vertex array as given 
	GL_Model, applying given shader following
	the transform standard.


*/
class dmodel : public drawable
{
public:

	transform tform;

	shader sh;

	virtual void draw() override;

	dmodel(shader sh);
	~dmodel();
};

