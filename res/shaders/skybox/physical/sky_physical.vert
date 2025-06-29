#version 410 core

layout (location = 0) in vec3 aPos;

out vec3 vWorldDir;

uniform mat4 projection;
uniform mat4 view;

void main() {
    vec4 pos = projection * mat4(mat3(view)) * vec4(aPos, 1.0);
    vWorldDir =  aPos; // strip translation
    //gl_Position = projection * vec4(aPos * 500, 1.0); // far away cube
    
    gl_Position = pos.xyww;
}
