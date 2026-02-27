#version 330 core

in vec4 color;
in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D u_texture;
uniform float amount;
uniform vec2 resolution;

void main()
{
    vec4 result;
    result.r = texture(u_texture, texCoord + vec2(amount, 0.0)).r;
    result.g = texture(u_texture, texCoord).g;
    result.b = texture(u_texture, texCoord - vec2(amount, 0.0)).b;
    result.a = texture(u_texture, texCoord).a;
    fragColor = result;
}
