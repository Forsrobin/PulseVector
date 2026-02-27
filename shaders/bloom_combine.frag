#version 330 core

in vec4 color;
in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D u_texture;
uniform sampler2D bloomTexture;
uniform float intensity;
uniform vec2 resolution;

void main()
{
    vec4 sceneColor = texture(u_texture, texCoord);
    vec4 bloomColor = texture(bloomTexture, texCoord);
    fragColor = sceneColor + bloomColor * intensity;
}
