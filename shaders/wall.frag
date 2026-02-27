#version 330 core

out vec4 FragColor;
in vec2 texCoord;

uniform float time;
uniform float amplitude;
uniform float bass;
uniform vec2 resolution;
uniform vec4 u_baseColor;

void main()
{
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    
    // Procedural "energy" effect for walls
    float scanline = sin(gl_FragCoord.y * 0.2 + time * 10.0) * 0.1;
    float pulse = sin(time * 5.0) * 0.1 + 0.9;
    
    vec3 color = u_baseColor.rgb;
    
    // Add some glow based on audio
    color += vec3(0.5, 0.8, 1.0) * bass * 0.4;
    
    // Horizontal energy bars
    float bar = step(0.98, fract(uv.y * 20.0 + time * 2.0));
    color += vec3(1.0) * bar * 0.3;

    FragColor = vec4(color * pulse + scanline, u_baseColor.a);
}
