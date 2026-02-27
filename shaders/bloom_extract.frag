#version 330 core

in vec4 color;
in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D u_texture;
uniform float threshold;

void main()
{
    vec4 texColor = texture(u_texture, texCoord);
    float brightness = dot(texColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > threshold)
        fragColor = texColor;
    else
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
