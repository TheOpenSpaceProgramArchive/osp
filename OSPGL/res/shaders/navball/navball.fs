#version 330
out vec4 FragColor;


in vec3 vColor;
in vec2 vTex;
in vec3 vNormal;
in vec3 vPos;
in vec3 vPosOriginal;

uniform sampler2D tex;

void main()
{
	vec3 norm = normalize(vNormal); 
	vec3 lightDir = normalize(vec3(0.7, 1.0, 0)); 
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1, 1, 1);
	diffuse = max(vec3(0.2, 0.2, 0.2), diffuse);

	// Mapping
	float pi = 3.14159265359;
	vec2 mapped = vec2(atan(-vPosOriginal.x, -vPosOriginal.z)/(2*pi) + 0.5, (asin(-vPosOriginal.y) / pi + 0.5));
    FragColor = vec4(vec3(texture(tex, mapped)) * diffuse, 1.0);
} 