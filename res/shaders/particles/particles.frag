#version 330 core
in vec2 fTexCoords;
in vec4 fColor;

layout(location = 0) out vec4 FragColor;    
layout(location = 1) out vec4 BrightColor;


uniform sampler2D particleTexture;
uniform float brightnessThreshold = 1.0;
uniform float emission = 1.0;

void main() {

    vec4 tex = texture(particleTexture, fTexCoords);
    
    if (tex.a < 0.1)
        discard;

    vec4 finalColor = vec4(fColor.rgb * tex.rgb * emission, fColor.a * tex.a);

    FragColor = finalColor;
    float lum = dot(finalColor.rgb, vec3(0.2126, 0.7152, 0.0722));

    if (lum > brightnessThreshold) {
        BrightColor = vec4(finalColor.rgb, 1.0);
    } else {
        BrightColor = vec4(0.0);
    }
}