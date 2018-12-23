#version 330
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec3 vPos;  

void main()
{
	vPos = vec3(vec4(aPos, 1.0));
	gl_Position = proj * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
} 