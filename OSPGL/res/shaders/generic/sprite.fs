#version 330
out vec4 FragColor;


in vec2 vTex;
in vec3 vCol;

uniform sampler2D tex;

void main()
{
    FragColor = vec4(texture(tex, vTex) * vec4(vCol, 1.0));
} 