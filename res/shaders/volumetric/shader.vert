#version 410 core
layout (location = 0) in vec3 aPos;

uniform mat4 model, view, projection;
out vec3 Position;
void main(){
    gl_Position = projection * view * model * vec4(aPos,1.0);
    
    Position = vec3(model * vec4(aPos, 1.0));
}
