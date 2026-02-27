#version 330 core

out vec4 FragColor;
in vec2 texCoord;

uniform float time;
uniform float amplitude;
uniform float bass;
uniform float fft[64];
uniform vec2 resolution;

void main()
{
    vec2 uv = texCoord;
    vec3 color = vec3(0.05, 0.05, 0.1); // Base dark navy

    // Pulsing grid effect
    float gridLine = 0.0;
    vec2 gridUV = uv * 10.0;
    vec2 grid = abs(fract(gridUV - 0.5) - 0.5) / fwidth(gridUV);
    float line = min(grid.x, grid.y);
    gridLine = 1.0 - min(line, 1.0);
    
    color += gridLine * vec3(0.0, 0.5, 1.0) * (0.1 + bass * 0.5);

    // Audio visualizer at bottom
    if (uv.y < 0.2) {
        int bin = int(uv.x * 64.0);
        float val = fft[bin] * 2.0;
        if (uv.y < val * 0.2) {
            color += vec3(0.0, 1.0, 1.0) * 0.5;
        }
    }

    // Centered pulse
    float dist = distance(uv, vec2(0.5));
    float pulse = exp(-dist * 10.0) * amplitude * 2.0;
    color += vec3(0.5, 0.0, 0.5) * pulse;

    FragColor = vec4(color, 1.0);
}
