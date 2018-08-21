#version 330
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aCol;
layout (location = 2) in vec2 aTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 vColor;
out vec2 vTex;

void main()
{
	vColor = aCol;
		vTex = aTex;
	gl_Position = proj * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);

} 