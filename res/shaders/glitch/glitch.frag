#version 330 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform float time;
uniform float glitchIntensity;
uniform float digitalNoiseIntensity;
uniform float rgbShiftIntensity;
uniform float blockIntensity;

// Random function
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

// Noise function
float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);
    
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));
    
    vec2 u = f * f * (3.0 - 2.0 * f);
    
    return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

void main() {
    vec2 uv = TexCoords;
    vec3 color = texture(screenTexture, uv).rgb;
        
    // Horizontal displacement based on time and noise
    float glitchTime = time * 0.5;
    float verticalGlitch = step(0.98, sin(uv.y * 50.0 + glitchTime * 2.0));
    float horizontalDisplacement = (random(vec2(floor(uv.y * 100.0), floor(glitchTime))) - 0.5) * glitchIntensity * verticalGlitch;
    
    // RGB shift effect
    vec2 rgbShiftOffset = vec2(rgbShiftIntensity * 0.01, 0.0);
    float r = texture(screenTexture, uv + horizontalDisplacement + rgbShiftOffset).r;
    float g = texture(screenTexture, uv + horizontalDisplacement).g;
    float b = texture(screenTexture, uv + horizontalDisplacement - rgbShiftOffset).b;
    
    // Block distortion
    vec2 blockUV = uv;
    float blockNoise = random(vec2(floor(uv.y * 20.0), floor(time * 2.0)));
    if (blockNoise > 0.8) {
        blockUV.x += (random(vec2(floor(uv.y * 20.0), floor(time * 2.0))) - 0.5) * blockIntensity;
    }
    
    // Digital noise
    float digitalNoise = random(uv + time) * digitalNoiseIntensity;
    
    // Scanlines
    float scanlines = sin(uv.y * 800.0) * 0.04;
    
    // Static noise
    float staticNoise = (random(uv + time * 2.0) - 0.5) * 0.1;
    
    // Combine effects
    vec3 finalColor = vec3(r, g, b);
    
    // Apply block distortion to final color
    if (blockNoise > 0.8) {
        finalColor = texture(screenTexture, blockUV).rgb;
    }
    
    // Add digital noise
    finalColor += digitalNoise;
    
    // Add scanlines
    finalColor += scanlines;
    
    // Add static noise
    finalColor += staticNoise;
    
    // Occasionally invert colors for extreme glitch effect
    float invertChance = step(0.99, random(vec2(floor(time * 10.0), 0.0)));
    if (invertChance > 0.0 && glitchIntensity > 0.5) {
        finalColor = 1.0 - finalColor;
    }
    
    // Color quantization for digital look
    finalColor = floor(finalColor * 8.0) / 8.0;
    
    FragColor = vec4(finalColor, 1.0);
}