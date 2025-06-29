#version 410 core
out vec4 FragColor;

uniform float u_Fade;       
uniform vec3 u_FadeColor = vec3(0.0); 

void main() {
    FragColor = vec4(u_FadeColor, u_Fade);
}