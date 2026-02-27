#version 330 core

out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D u_texture;
uniform vec2 center;
uniform float strength;
uniform vec2 resolution;

void main()
{
    vec2 uv = texCoord;
    vec2 dir = uv - center / resolution;
    vec4 color = vec4(0.0);
    float samples = 10.0;

    for (float i = 0.0; i < samples; i++) {
        color += texture(u_texture, uv - dir * strength * (i / samples));
    }

    FragColor = color / samples;
}
