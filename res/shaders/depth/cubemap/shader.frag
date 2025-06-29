#version 410 core
in vec4 FragPos;

uniform vec3 lightPos;
uniform float farPlane;


void main()
{
    // Manually sets depth map in the range [0, 1]
	float distanceToLight = length(FragPos.xyz - lightPos);

    gl_FragDepth = distanceToLight / farPlane;
}