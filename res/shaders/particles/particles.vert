#version 330 core
layout(location = 0) in vec3 aPos;       
layout(location = 1) in vec2 aTexCoords;   

layout(location = 2) in vec3 instanceOffset;
layout(location = 3) in float instanceSize;  
layout(location = 4) in vec4 instanceColor;  

out vec2 fTexCoords;
out vec4 fColor;

uniform mat4 view;
uniform mat4 projection;

uniform bool u_BillboardY = true;

void main() {
    vec3 camRight = vec3(view[0][0], 0.0, view[2][0]);  
    camRight = normalize(camRight);
    vec3 camUp = vec3(0.0, 1.0, 0.0); 

    vec3 pos = aPos * instanceSize;

    mat4 model;

    if (u_BillboardY) {
        model = mat4(
            vec4(camRight, 0.0),
            vec4(camUp,    0.0),
            vec4(0.0, 0.0, 0.0, 0.0),
            vec4(instanceOffset, 1.0)
        );
        vec4 worldPos = vec4(instanceOffset + camRight * pos.x + camUp * pos.y, 1.0);
        gl_Position = projection * view * worldPos;
    } else {
        model = mat4(
            vec4(vec3(view[0][0], view[1][0], view[2][0]) * instanceSize, 0.0),
            vec4(vec3(view[0][1], view[1][1], view[2][1]) * instanceSize, 0.0),
            vec4(0.0, 0.0, 0.0, 1.0),
            vec4(instanceOffset, 1.0)
        );
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }

    fTexCoords = aTexCoords;
    fColor = instanceColor;
}