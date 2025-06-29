#version 410 core
uniform samplerCubeArray cubemap;
uniform int faceIndex; // 0 to 5 (positive_x to negative_z)
uniform int layer;

in vec2 TexCoords;
out vec4 FragColor;

vec3 GetDirectionFromFace(vec2 uv, int face)
{
    uv = uv * 2.0 - 1.0; // Map [0,1] to [-1,1]
    vec3 dir;
    if (face == 0) dir = vec3( 1.0, -uv.y, -uv.x); // +X
    if (face == 1) dir = vec3(-1.0, -uv.y,  uv.x); // -X
    if (face == 2) dir = vec3( uv.x,  1.0,  uv.y); // +Y
    if (face == 3) dir = vec3( uv.x, -1.0, -uv.y); // -Y
    if (face == 4) dir = vec3( uv.x, -uv.y,  1.0); // +Z
    if (face == 5) dir = vec3(-uv.x, -uv.y, -1.0); // -Z
    return normalize(dir);
}

void main()
{
    vec3 direction = GetDirectionFromFace(TexCoords, faceIndex);
    float depth = texture(cubemap, vec4(direction, float(layer))).r;
    FragColor =  vec4(vec3(depth), 1.0); // For depth cubemap
}