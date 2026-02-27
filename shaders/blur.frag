#version 330 core

in vec4 color;
in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D u_texture;
uniform vec2 direction;
uniform vec2 resolution;

void main()
{
    float weights[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
    vec2 off = direction / resolution;
    
    vec4 result = texture(u_texture, texCoord) * weights[0];
    for (int i = 1; i < 5; ++i)
    {
        result += texture(u_texture, texCoord + off * float(i)) * weights[i];
        result += texture(u_texture, texCoord - off * float(i)) * weights[i];
    }
    fragColor = result;
}
