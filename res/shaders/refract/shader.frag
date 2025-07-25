#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;

uniform float coefR;
uniform float coefG;
uniform float coefB;
uniform vec3 cameraPos;
uniform samplerCube skybox;

    void main()
    {             
        float ratio = 1.00 / 1.05;
        vec3 I = normalize(Position - cameraPos);
        vec3 R = refract(I, normalize(Normal), ratio);
        FragColor = vec4(texture(skybox, R).rgb, 1.0);
    } 