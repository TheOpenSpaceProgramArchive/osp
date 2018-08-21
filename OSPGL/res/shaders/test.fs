#version 330
out vec4 FragColor;


in vec3 vColor;
in vec2 vTex;

void main()
{
    FragColor = vec4(vColor, 1.0f);
} 