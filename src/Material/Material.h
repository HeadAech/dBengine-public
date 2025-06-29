
#ifndef MATERIAL_H
#define MATERIAL_H


#include <glad/glad.h>
#include <string>
#include <Shader/Shader.h>
#include "Helpers/UUID/UUID.h"
#include <memory>

/// <summary>
/// Object holding necessary information about a texture.
/// <para> ID - OpenGL texture ID </para>
/// <para> type - Type of the texture (e.g., diffuse, normal) </para>
/// <para> path - Path to the texture file </para>
/// </summary>
struct Texture {
    GLuint id;
    std::string type;
    std::string path;
};

/// <summary>
/// Defines the type of material.
/// </summary>
enum class MaterialType { PBR, LIGHT, BLINN_PHONG, REFRACTION, REFLECTION };

/// <summary>
/// Class representing a material in the rendering engine.
/// </summary>
class Material {
public:

    /// <summary>
    /// UUID of the material.
    /// </summary>
    std::string UUID = UUID::generateUUID();

    /// <summary>
    /// Name of the material.
    /// </summary>
    std::string name;

    /// <summary>
    /// Type of the material.
    /// </summary>
    MaterialType type = MaterialType::PBR;

	std::shared_ptr<Texture> diffuse = std::make_shared<Texture>();
    std::shared_ptr<Texture> normal = std::make_shared<Texture>();
    std::shared_ptr<Texture> metallic = std::make_shared<Texture>();
    std::shared_ptr<Texture> roughness = std::make_shared<Texture>();
    std::shared_ptr<Texture> ambientOcclusion = std::make_shared<Texture>();
    std::shared_ptr<Texture> height = std::make_shared<Texture>();
    std::shared_ptr<Texture> specular = std::make_shared<Texture>();
    std::shared_ptr<Texture> emissive = std::make_shared<Texture>();

    /// <summary>
    /// Defines if the material is using Triplanar scaling.
    /// </summary>
    bool Triplanar = false;

    /// <summary>
    /// Sets the Triplanar tiling scale.
    /// </summary>
    float TriplanarTilingScale = 1.0;

    Material();
    ~Material() = default;

    /// <summary>
    /// Binds material's textures to the shader.
    /// </summary>
    /// <param name="shader">Pointer to target shader</param>
    void Bind(Shader *shader);
    
    /// <summary>
    /// Returns the material type as an integer.
    /// </summary>
    /// <returns>Enum (MaterialType)</returns>
    int GetMaterialType();

    /// <summary>
    /// Sets the material type.
    /// </summary>
    /// <param name="type">Enum (MaterialType)</param>
    void SetMaterialType(int type);


    /// <summary>
    /// Returns true if the material is empty (no textures).
    /// </summary>
    /// <returns>Boolean</returns>
    bool IsEmpty();

    /// <summary>
    /// Returns the name of the material type as a string.
    /// </summary>
    /// <returns>Name (String)</returns>
    std::string GetMaterialTypeName();


    /// <summary>
    /// Sets a texture to the material based on the property name.
    /// </summary>
    /// <param name="pTexture">Shared pointer to a texture</param>
    /// <param name="propertyName">Name of the property</param>
    void SetTexture(std::shared_ptr<Texture> pTexture, std::string_view propertyName);
};


#endif // !MATERIAL_H
