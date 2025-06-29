#version 410 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aWeights;
layout (location = 7) in mat4 aInstanceMatrix; // spans 7–10
layout(location = 11) in int aInstanceVisible;

#define MAX_SHADOW_CASTERS 64

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    //vec4 FragPosLightSpaces[MAX_SHADOW_CASTERS];
    mat3 TBN;
    flat int visible;
} vs_out;

uniform mat4 projection;
uniform mat4 view;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;

uniform bool isAnimated = false;

layout(std140) uniform Bones {
    mat4 finalBonesMatrices[MAX_BONES];
};

void main() 
{

    mat3 transposed = transpose(inverse(mat3(aInstanceMatrix)));
    vec3 T = normalize(transposed * aTangent);

    float det = determinant(mat3(aInstanceMatrix));

    vec3 N = normalize(transposed * aNormal);

    vs_out.FragPos = vec3(aInstanceMatrix * vec4(aPos, 1.0));
    
    vs_out.TexCoords = aTexCoords;


    vec4 totalPosition = vec4(aPos, 1.0f);
    vec3 totalNormal = aNormal;

    if (isAnimated) {
        totalPosition = vec4(0.0f);
        totalNormal = vec3(0.0f);
        for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
        {
            if(aBoneIDs[i] == -1) 
                continue;
            if(aBoneIDs[i] >=MAX_BONES) 
            {
                totalPosition = vec4(aPos,1.0f);
                totalNormal = aNormal;
                break;
            }
            vec4 localPosition = finalBonesMatrices[aBoneIDs[i]] * vec4(aPos,1.0f);
            totalPosition += localPosition * aWeights[i];
            vec3 localNormal = mat3(finalBonesMatrices[aBoneIDs[i]]) * aNormal;
            totalNormal += localNormal * aWeights[i];
        }
        totalNormal = normalize(totalNormal); // Normalize the accumulated normal
        N = normalize(transposed * totalNormal);
    }

    vs_out.Normal = N;
    T = normalize(T - dot(T, N) * N); // Gram-Schmidt orthogonalization
    vec3 B = normalize(transposed * aBitangent);
    mat3 TBN = mat3(T, B, N);
    vs_out.TBN = TBN;

    mat4 viewModel = view * aInstanceMatrix;
    gl_Position = projection * viewModel * totalPosition;
}