#version 410 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D depthMap;
uniform sampler2DArray depthMapArray;
uniform int debugLayer = 0;
uniform int testArray = 1;

void main()
{
    if (testArray == 1) {
         float depth = texture(depthMapArray, vec3(TexCoords, debugLayer)).r;
        FragColor = vec4(vec3(depth), 1.0); // szaroœæ na podstawie g³êbokoœci
    } else {
        float depthValue = texture(depthMap, TexCoords).r;
        FragColor = vec4(vec3(depthValue), 1.0); // grayscale
    }
}