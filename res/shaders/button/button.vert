#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform vec2 u_Position;
uniform vec2 u_Size;
uniform vec2 u_ScreenResolution;

out vec2 TexCoords;

void main() {
    vec2 scaled = aPos * u_Size;
    vec2 worldPos = scaled + u_Position;
    vec2 ndc = (worldPos / u_ScreenResolution) * 2.0 - 1.0;
   
    gl_Position = vec4(ndc.x, ndc.y, 0.0, 1.0);

    TexCoords = aTexCoords;
}