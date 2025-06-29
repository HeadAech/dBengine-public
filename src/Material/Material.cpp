#include "Material.h"

Material::Material() {
    name = "Material";
}

void Material::Bind(Shader *shader) { 
    shader->Use(); 

    // bind diffuse map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse->path.empty() ? 0 : diffuse->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // bind normal map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normal->path.empty() ? 0 : normal->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // bind metallic map
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, metallic->path.empty() ? 0 : metallic->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // bind roughness map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, roughness->path.empty() ? 0 : roughness->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // bind ambient occlusion map
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, ambientOcclusion->path.empty() ? 0 : ambientOcclusion->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // bind specular map
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, specular->path.empty() ? 0 : specular->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // bind height map
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, height->path.empty() ? 0 : height->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // bind emissive map
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, emissive->path.empty() ? 0 : emissive->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    shader->SetBool("u_Triplanar", Triplanar);
    if (Triplanar)
    {
        shader->SetFloat("u_TriplanarRepeat", TriplanarTilingScale);
    }

}

void Material::SetMaterialType(int type) {
    this->type = static_cast<MaterialType>(type);
}

int Material::GetMaterialType() { 
    return (int)this->type; 
}

bool Material::IsEmpty() {
    return (diffuse->path.empty() && normal->path.empty() && metallic->path.empty() && roughness->path.empty() &&
            ambientOcclusion->path.empty() && specular->path.empty() && height->path.empty() && emissive->path.empty());
}

std::string Material::GetMaterialTypeName()
{
    switch (type)
    {
        case MaterialType::PBR:
            return "PBR";
        case MaterialType::LIGHT:
            return "Light";
        case MaterialType::REFRACTION:
            return "Refractive";
        case MaterialType::REFLECTION:
            return "Reflective";
        default:
            return "No type definition";
    }
}

void Material::SetTexture(std::shared_ptr<Texture> pTexture, std::string_view propertyName)
{
    if (propertyName == "Diffuse")
    {
        diffuse = pTexture;
    }
    else if (propertyName == "Normal")
    {
        normal = pTexture;
    }
    else if (propertyName == "Metallic")
    {
        metallic = pTexture;
    }
    else if (propertyName == "Roughness")
    {
        roughness = pTexture;
    }
    else if (propertyName == "Ambient Occlusion")
    {
        ambientOcclusion = pTexture;
    }
    else if (propertyName == "Specular")
    {
        specular = pTexture;
    }
    else if (propertyName == "Height")
    {
        height = pTexture;
    }
    else if (propertyName == "Emissive")
    {
        emissive = pTexture;
    }
}
