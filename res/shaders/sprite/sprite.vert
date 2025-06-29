#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform vec2 u_Position;
uniform vec2 u_Size;
uniform vec2 u_ScreenResolution;
uniform float u_Rotation = 0.0;

out vec2 TexCoords;

void main() {
    vec2 scaled = aPos * u_Size;

    vec2 center = u_Position + 0.5 * u_Size;
    vec2 toRotate = scaled - 0.5 * u_Size;

    float rad = radians(u_Rotation);
    float cosTheta = cos(rad);
    float sinTheta = sin(rad);
    vec2 rotated;
    rotated.x = toRotate.x * cosTheta - toRotate.y * sinTheta;
    rotated.y = toRotate.x * sinTheta + toRotate.y * cosTheta;

    vec2 worldPos = center + rotated;
    vec2 ndc = (worldPos / u_ScreenResolution) * 2.0 - 1.0;
   
    gl_Position = vec4(ndc.x, ndc.y, 0.0, 1.0);

    TexCoords = aTexCoords;
}