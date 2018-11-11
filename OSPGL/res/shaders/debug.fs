#version 330
out vec4 FragColor;

uniform vec4 color;

in vec3 vPos;

void main()
{
    FragColor = color;
} 