#version 330
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec3 aNrm;
layout (location = 3) in vec2 aTex;

out vec2 vTex;
out vec3 vCol;

uniform mat4 view;
uniform mat4 proj;
uniform mat4 model;

void main()
{
	vTex = aTex;
	vCol = aCol;
	gl_Position = proj * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
} 