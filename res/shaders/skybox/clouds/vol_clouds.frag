#version 410 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 sunDirection;
uniform float time;
uniform vec3 cameraPos;
uniform mat4 invProjection;
uniform mat4 invView;

// Cloud parameters
uniform float cloudLayerHeight;  // Starting height for clouds
uniform float cloudThickness;    // Vertical extent of clouds
uniform float cloudScale;        // Scale factor for noise
uniform int numSteps;            // Raymarching iterations
uniform float stepSize;          // Distance between steps

// Simple pseudo-random noise function based on position
float noise(vec3 p) {
    return fract(sin(dot(p, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
}

// Fractal Brownian motion for smoother noise (5 octaves)
float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    for(int i = 0; i < 5; i++) {
        value += amplitude * noise(p * frequency);
        frequency *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

// Compute cloud density at a given world-space position
float computeCloudDensity(vec3 pos) {
    // Offset the y coordinate so the noise is centered on the cloud layer
    pos.y -= cloudLayerHeight;
    // Sample fBm noise and animate it (simulate wind)
    float density = fbm(pos * cloudScale + vec3(0.0, time * 0.05, 0.0));
    // Use smoothstep to threshold and sharpen the clouds
    density = smoothstep(0.5, 0.7, density);
    return density;
}

void main() {
    // Reconstruct view-space ray direction for this fragment
    // Convert TexCoords from [0,1] to NDC coordinates [-1,1]
    vec2 ndc = TexCoords * 2.0 - 1.0;
    vec4 clipPos = vec4(ndc, 0.0, 1.0);
    vec4 viewPos = invProjection * clipPos;
    viewPos /= viewPos.w;
    vec3 rayDir = normalize((invView * vec4(viewPos.xyz, 0.0)).xyz);

    // Initialize the ray start position at the camera
    vec3 rayOrigin = cameraPos;
    
    // Accumulators for the cloud color and remaining transmittance
    vec3 cloudColor = vec3(0.0);
    float transmittance = 1.0;
    float totalDistance = 0.0;
    
    // Simple raymarching loop
    for (int i = 0; i < numSteps; i++) {
        vec3 samplePos = rayOrigin + rayDir * totalDistance;
        
        // Only march when within the cloud layer (optional: restrict to cloud region)
        if (samplePos.y > cloudLayerHeight && samplePos.y < (cloudLayerHeight + cloudThickness)) {
            float density = computeCloudDensity(samplePos);
            
            // Compute a simple lighting model: dot product between sun and ray direction
            float lightIntensity = max(dot(normalize(sunDirection), rayDir), 0.0);
            vec3 sampleColor = vec3(1.0) * lightIntensity;
            
            // Compute absorption using an exponential falloff based on density
            float absorption = 1.0 - exp(-density * stepSize);
            cloudColor += sampleColor * absorption * transmittance;
            
            // Update the transmittance (how much light is not absorbed)
            transmittance *= exp(-density * stepSize);
        }
        
        totalDistance += stepSize;
        
        // Early exit if fully opaque
        if(transmittance < 0.01)
            break;
    }

    // Blend the accumulated cloud color with a background sky color
    vec3 skyColor = vec3(0.2, 0.4, 1.0); // Replace with your skybox or atmosphere color if needed
    vec3 finalColor = mix(skyColor, cloudColor, 1.0 - transmittance * 0.5);
    FragColor = vec4(finalColor, 1.0);
}
