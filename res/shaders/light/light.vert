#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
//layout (location = 5) in ivec4 aBoneIDs;
//layout (location = 6) in vec4 aWeights;
layout (location = 7) in mat4 aInstanceMatrix; // spans 7–10

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    //vec4 FragPosLightSpaces[MAX_SHADOW_CASTERS];
} vs_out;

uniform mat4 projection;
uniform mat4 view;


void main()
{
    vs_out.FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));
    vs_out.Normal = mat3(transpose(inverse(aInstanceMatrix))) * aNormal;
    vs_out.TexCoords = aTexCoords;
	mat3 transposed = transpose(inverse(mat3(aInstanceMatrix)));
    //vec3 T = normalize(transposed * aTangent);
    //vec3 N = normalize(transposed * aNormal);
    //T = normalize(T - dot(T, N) * N); // Gram-Schmidt orthogonalization
    //vec3 B = cross(N, T);
    //mat3 TBN = mat3(T, B, N);
    //vs_out.TBN = TBN;
    //calculate light positions for all casters
    //for (int i = 0; i < MAX_SHADOW_CASTERS; ++i)
       // vs_out.FragPosLightSpaces[i] = lightSpaceMatrices[i] * vec4(vs_out.FragPos, 1.0);


    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);
}