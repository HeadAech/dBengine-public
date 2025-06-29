#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <Helpers/UUID/UUID.h>

/// <summary>
/// Class responsible for loading, compiling, and using shaders in the engine.
/// </summary>
class Shader
{
public:
    unsigned int ID;
    /// <summary>
    /// Constructor that initializes the shader program from vertex and fragment shader files.
    /// </summary>
    /// <param name="vertexPath">Path to the vertex shader</param>
    /// <param name="fragmentPath">Path to the fragment shader</param>
    /// <param name="geometryPath">(Optional) Path to the geometry shader</param>
    Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
    Shader() = default;
    ~Shader();

    /// <summary>
    /// Makes the shader program active for rendering.
    /// </summary>
    void Use();

    /// <summary>
    /// Sets a boolean uniform variable in the shader.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="value">Boolean value</param>
    void SetBool(const std::string &name, bool value) const;

    /// <summary>
    /// Sets an integer uniform variable in the shader.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="value">Integer value</param>
    void SetInt(const std::string &name, int value) const;

    /// <summary>
    /// Sets a 2D sampler uniform variable in the shader, typically used for textures.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="textureID">ID of the texture</param>
    void SetSampler2D(const std::string &name, unsigned int textureID) const;

    /// <summary>
    /// Sets a float uniform variable in the shader.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="value">Float value</param>
    void SetFloat(const std::string &name, float value) const;

    /// <summary>
    /// Sets a 2D vector uniform variable in the shader.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="value">Vec2 value</param>
    void SetVec2(const std::string &name, const glm::vec2 &value) const;

    /// <summary>
    /// Sets a 2D vector uniform variable in the shader with separate x and y components.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="x">X value (float)</param>
    /// <param name="y">Y value (float)</param>
    void SetVec2(const std::string &name, float x, float y) const;

    /// <summary>
    /// Sets a 3D vector uniform variable in the shader.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="value">Vec3 value</param>
    void SetVec3(const std::string &name, const glm::vec3 &value) const;

    /// <summary>
    /// Sets a 3D vector uniform variable in the shader with separate x, y and z components.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="x">X value (float)</param>
    /// <param name="y">Y value (float)</param>
    /// <param name="z">Z value (float)</param>
    void SetVec3(const std::string &name, float x, float y, float z) const;

    /// <summary>
    /// Sets a 4D vector uniform variable in the shader.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="value">Vec4 value</param>
    void SetVec4(const std::string &name, const glm::vec4 &value) const;

    /// <summary>
    /// Sets a 4D vector uniform variable in the shader with separate x, y, z and w components.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="x">X value (float)</param>
    /// <param name="y">Y value (float)</param>
    /// <param name="z">Z value (float)</param>
    /// <param name="w">W value (float)</param>
    void SetVec4(const std::string &name, float x, float y, float z, float w) const;

    /// <summary>
    /// Sets a 2x2 matrix uniform variable in the shader.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="mat">2x2 Matrix</param>
    void SetMat2(const std::string &name, const glm::mat2 &mat) const;
   
    /// <summary>
    /// Sets a 3x3 matrix uniform variable in the shader.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="mat">3x3 Matrix</param>
    void SetMat3(const std::string &name, const glm::mat3 &mat) const;

    /// <summary>
    /// Sets a 4x4 matrix uniform variable in the shader.
    /// </summary>
    /// <param name="name">Name of uniform</param>
    /// <param name="mat">4x4 Matrix</param>
    void SetMat4(const std::string &name, const glm::mat4 &mat) const;

    /// <summary>
    /// Reloads the shader program from the specified vertex and fragment shader files.
    /// </summary>
    void Reload();



private:

    void checkCompileErrors(GLuint shader, std::string type);

    GLuint loadFromFile(const char *vertexPath, const char *fragmentPath, const char *geometryPath = nullptr);

    std::string vertexPath;
    std::string fragmentPath;
    std::string geometryPath;
    std::string uuid;
};
#endif