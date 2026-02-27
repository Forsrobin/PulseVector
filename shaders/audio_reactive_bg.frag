#version 330 core

out vec4 FragColor;
in vec2 texCoord;

uniform float time;
uniform float amplitude;
uniform float bass;
uniform float u_dimAmount;
uniform float fft[64];
uniform vec2 resolution;

void main()
{
    // Use texture coordinates from vertex shader if resolution is zero
    vec2 uv = (resolution.x > 0.0) ? (gl_FragCoord.xy / resolution.xy) : texCoord;
    
    vec3 color = vec3(0.05, 0.05, 0.1); 

    // Background simple pulse
    float pulse = 0.5 + 0.5 * sin(time * 2.0);
    color += vec3(0.0, 0.2, 0.4) * pulse * (0.5 + bass);
    
    // Tiny amplitude influence
    color += vec3(0.2, 0.0, 0.2) * amplitude;

    FragColor = vec4(color * u_dimAmount, 1.0);
}
