#version 330
out vec4 FragColor;


in vec2 vTex;
in vec3 vNormal;
in vec3 vPos;

void main()
{

	vec3 norm = normalize(vNormal); 
	vec3 lightDir = normalize(vec3(1, 0, 0)); 
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * vec3(1, 1, 1);
	diffuse = max(vec3(0.01, 0.01, 0.01), diffuse);

    vec3 col = vec3(0.9, 0.8, 0.6);

    FragColor = vec4(col * diffuse, 1.0);
    //FragColor = vec4(norm.x, norm.y, norm.z, 1.0);
} 