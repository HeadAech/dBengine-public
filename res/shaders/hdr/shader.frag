#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float exposure;
uniform sampler2D hdrBuffer;

uniform sampler2D bloomBlur;
uniform float bloomStrength = 0.04f;

// Camera shake uniforms
uniform float time;         // Current time, passed from CPU
uniform float shakeAmount;  // Intensity of shake, e.g. 0.0 (off) to 0.05 (strong)

uniform float u_VignetteStrength = 0.5;

uniform float glitchIntensity = 0.1;
uniform float glitchFrequency = 40.0; 
uniform float u_Time;
uniform float segmentFrequency = 20.0;

uniform bool u_EnableRedBars = false;

uniform bool u_EnableGlitchEffect = false;

// Random generator for shake
float rand(vec2 co) {
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}
float noise(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898,78.233))) * 43758.5453);
}


vec3 bloom(vec2 coords) {
    vec3 hdrColor = texture(hdrBuffer, coords).rgb;
    vec3 bloomColor = texture(bloomBlur, coords).rgb;
    return mix(hdrColor, bloomColor, bloomStrength); // linear interpolation
}

void main()
{             
    const float gamma = 2.2;
    
    // Calculate shake offsets
    float shakeX = (rand(vec2(time, 0.0)) * 2.0 - 1.0) * shakeAmount;
    float shakeY = (rand(vec2(0.0, time)) * 2.0 - 1.0) * shakeAmount;

    // Apply shake to texture coordinates
    vec2 shakenCoords = TexCoords + vec2(shakeX, shakeY);

    // Compute noise-based glitch factor
    float glitchFactor = 0.0;
    float segmentNoise = 0.0;
    
    // Apply glitch more strongly near the edges
    float edgeDistance = min(min(TexCoords.x, 1.0 - TexCoords.x), min(TexCoords.y, 1.0 - TexCoords.y));
    float edgeFactor = smoothstep(0.0, 0.2, edgeDistance);
    
    float offsetR = 0.0;
    float offsetG = 0.0;
    float offsetB = 0.0;

    // Use noise to create random bars
    if (u_EnableGlitchEffect)
    {
        float noiseValue = noise(vec2(floor(TexCoords.y * glitchFrequency), u_Time));
        segmentNoise = noise(vec2(floor(TexCoords.x * segmentFrequency), floor(TexCoords.y * glitchFrequency)));
        glitchFactor = step(0.2, noiseValue) * (1.0 - edgeFactor);
    
        // Compute glitch animation parameter
        float t = floor(u_Time * 10.0); // Changes every 0.1 seconds for dynamic effect

        // Compute horizontal offsets for each color channel (chromatic aberration)
        offsetR = glitchFactor * (rand(vec2(TexCoords.y + t + 0.1, 0.0)) * 2.0 - 1.0) * glitchIntensity;
        offsetG = glitchFactor * (rand(vec2(TexCoords.y + t + 0.2, 0.0)) * 2.0 - 1.0) * glitchIntensity;
        offsetB = glitchFactor * (rand(vec2(TexCoords.y + t + 0.3, 0.0)) * 2.0 - 1.0) * glitchIntensity;
    }

    // Adjusted UV coordinates for each color channel
    vec2 uvR = shakenCoords + vec2(offsetR, 0.0);
    vec2 uvG = shakenCoords + vec2(offsetG, 0.0);
    vec2 uvB = shakenCoords + vec2(offsetB, 0.0);

    // Sample hdrBuffer with per-channel offsets
    vec3 hdrColor;
    hdrColor.r = texture(hdrBuffer, uvR).r;
    hdrColor.g = texture(hdrBuffer, uvG).g;
    hdrColor.b = texture(hdrBuffer, uvB).b;

    // Sample bloomBlur with per-channel offsets
    vec3 bloomColor;
    bloomColor.r = texture(bloomBlur, uvR).r;
    bloomColor.g = texture(bloomBlur, uvG).g;
    bloomColor.b = texture(bloomBlur, uvB).b;

    // Combine base color and bloom
    vec3 result = mix(hdrColor, bloomColor, bloomStrength);

    // Apply random red glitch bars with horizontal segmentation
    if (u_EnableRedBars && u_EnableGlitchEffect)
    {
        float redGlitchNoise = noise(vec2(floor(TexCoords.y * glitchFrequency), u_Time + 100.0));
        float redGlitchStrength = step(0.8, redGlitchNoise) * step(0.5, segmentNoise) * glitchFactor;
        result.g = mix(result.g, 0.0, redGlitchStrength);
        result.b = mix(result.b, 0.0, redGlitchStrength);
     }
    // Exposure tone mapping
    result = vec3(1.0) - exp(-result * exposure);
    
    // Gamma correction
    result = pow(result, vec3(1.0 / gamma));

    // Apply vignette effect
    vec2 center = vec2(0.5);
    float d = length(TexCoords - center);
    float vignette = smoothstep(1.0, u_VignetteStrength, d);
    result *= vignette;
  
    FragColor = vec4(result, 1.0);
}