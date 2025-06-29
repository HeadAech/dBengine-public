#version 410 core

in vec3 vWorldDir;
out vec4 FragColor;

uniform vec3 sunDirection;
uniform vec3 sunColor;
uniform float timeOfDay;

const float PI = 3.14159265;

vec3 calculateSkyColor(vec3 rayDir, vec3 sunDir) {
    float cosTheta = dot(normalize(rayDir), normalize(sunDir));
    
    float mie = pow(max(cosTheta, 0.0), 60.0);
    float rayleigh = 0.5 + 0.5 * cosTheta;

    vec3 rayleighColor = vec3(0.2, 0.4, 1.0);
    vec3 mieColor = vec3(1.0, 0.9, 0.6);

    return rayleigh * rayleighColor + mie * mieColor;
}

void main() {
	float sunSize = 0.04;
	float sunFallOff = 0.005;
	
    vec3 rayDir = normalize(vWorldDir);
    
    // Compute base sky color
    vec3 baseColor = calculateSkyColor(rayDir, sunDirection);
    
    // Determine angular difference between the view ray and the sun direction
    float cosAngle = dot(rayDir, normalize(sunDirection));
    // We can compute the angle if needed: float angle = acos(clamp(cosAngle, -1.0, 1.0));
    
    // Instead of explicitly computing the angle, derive a threshold from the cosine:
    // Since increases, we can compute the cosine threshold for the sun's angular size.
    float cosThreshold = cos(sunSize);
    
    // Create a mask that is 1.0 if the ray is within the sun disc and falls smoothly to 0 outside
    // The smoothstep function gives a smooth transition.
    // Adjust the inner and outer edges as needed.
    float sunMask = smoothstep(cosThreshold, cosThreshold + sunFallOff, cosAngle);
    
    // Blend the sun's contribution: you could simply add a burst of color:
    // Here we boost the brightness of the sun area.
    vec3 sunEffect = vec3(1.0) * sunMask;
    
    vec3 finalColor = baseColor + sunEffect;
    
    FragColor = vec4(finalColor, 1.0);
}