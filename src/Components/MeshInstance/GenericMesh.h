#include <glad/glad.h>
#ifndef GENERIC_MESH_H
#define GENERIC_MESH_H

class GenericMesh
{
	public:

		GLuint VAO, VBO, EBO;

		void InitCube();
		void Draw() const;
		void Cleanup();
};

#endif // !GENERIC_MESH_H
