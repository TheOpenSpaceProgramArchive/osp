#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

uniform mat4 nview;

uniform vec3 viewdir;

out vec3 vColor;
out vec2 vTex;
out mat4 viewmat;
out vec3 viewDir;

void main()
{
	vColor = aCol;
	gl_Position = proj * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	viewmat = nview;
	viewDir = viewdir;
	vTex = aTex;
} 