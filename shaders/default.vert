#version 330 core

in vec2 sf_GPU_Vertex;
in vec4 sf_GPU_Color;
in vec2 sf_GPU_TexCoord;

out vec4 color;
out vec2 texCoord;

uniform mat4 sf_GPU_ModelViewProjectionMatrix;

void main()
{
    gl_Position = sf_GPU_ModelViewProjectionMatrix * vec4(sf_GPU_Vertex, 0.0, 1.0);
    color = sf_GPU_Color;
    texCoord = sf_GPU_TexCoord;
}
