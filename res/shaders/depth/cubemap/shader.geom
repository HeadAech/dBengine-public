#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadowMatrices[6];
//uniform int face;
uniform int lightIndex;

out vec4 FragPos;

void main()
{
    for(int face = 0; face < 6; face++)
    {
        // Sets the face we are currently working on
        gl_Layer = lightIndex * 6 + face;
        for(int i = 0; i < 3; i++)
        {
            // Make transformed vertex
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }
        EndPrimitive();
    }
}