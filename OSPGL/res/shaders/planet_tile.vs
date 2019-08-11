#version 330
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNrm;
layout (location = 2) in vec2 aTex;


uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

out vec2 vTex;
out vec3 vNormal;
out vec3 vPos;  

void main()
{
	vTex = aTex;
	//vNormal = mat3(transpose(inverse(model))) * aNrm;
	vNormal = aNrm;
	vPos = vec3(model * vec4(aPos, 1.0));
	gl_Position = proj * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);

} 