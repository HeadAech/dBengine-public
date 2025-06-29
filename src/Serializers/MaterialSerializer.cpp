#include "MaterialSerializer.h"

namespace Serialize {

    

    
    MaterialSerializer::MaterialSerializer() {}

    /// <summary>
    /// Returns an instance of materialSerializer
    /// </summary>
    /// <returns></returns>
    MaterialSerializer &MaterialSerializer::GetInstance() {
        static MaterialSerializer instance;
        return instance;
    }

    /// <summary>
    /// sets currently used material for de/serialization
    /// </summary>
    /// <param name="material"></param>
    void MaterialSerializer::SetMaterial(Material *material) { this->material = material; }
    
    /// <summary>
    /// serializes material to given file
    /// </summary>
    /// <param name="filepath"></param>
    void MaterialSerializer::Serialize(const std::string &filepath, Material* pMaterial) {
        SetMaterial(pMaterial);
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "materialName";
        out << YAML::Value << material->name;
        out << YAML::Key << "uuid";
        out << YAML::Value << material->UUID;
        out << YAML::Key << "materialType";
        out << YAML::Value << material->GetMaterialType();

        out << YAML::Key << "Textures";
        out << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "diffuse";
        out << YAML::Value << *material->diffuse;
        out << YAML::Key << "normal";
        out << YAML::Value << *material->normal;
        out << YAML::Key << "metallic";
        out << YAML::Value << *material->metallic;
        out << YAML::Key << "roughness";
        out << YAML::Value << *material->roughness;
        out << YAML::Key << "ambientOcclusion";
        out << YAML::Value << *material->ambientOcclusion;
        out << YAML::Key << "height";
        out << YAML::Value << *material->height;
        out << YAML::Key << "specular";
        out << YAML::Value << *material->specular;
        out << YAML::Key << "emissive";
        out << YAML::Value << *material->emissive;
        out << YAML::EndMap;

        out << YAML::Key << "triplanar";
        out << YAML::Value << material->Triplanar;
        out << YAML::Key << "triplanarTilingScale";
        out << YAML::Value << material->TriplanarTilingScale;

        out << YAML::EndMap;
        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    /// <summary>
    /// Deserialize data to currently set material
    /// </summary>
    /// <param name="filepath"></param>
    /// <returns></returns>
    bool MaterialSerializer::Deserialize(const std::string &filepath, Material* material) {
        SetMaterial(material);
        std::ifstream stream(filepath);
        std::stringstream strStream;
        strStream << stream.rdbuf();

        YAML::Node data = YAML::Load(strStream.str());
        if (!data["materialName"] || !data["uuid"] || !data["Textures"])
            return false;

        material->name = data["materialName"].as<std::string>();
        material->UUID = data["uuid"].as<std::string>();
        material->SetMaterialType(data["materialType"].as<int>());

        auto textures = data["Textures"];
        if (textures && textures.IsMap()) {
            for (const auto &pair: textures) {
                std::string textureKey = pair.first.as<std::string>();
                Texture texture = pair.second.as<Texture>();
                std::shared_ptr<Texture> pTexture = ResourceManager::GetInstance().LoadTextureFromFile(texture.path);
                    if (textureKey == "diffuse") {
                        material->diffuse = pTexture;
                    } else if (textureKey == "normal") {
                        material->normal = pTexture;
                    } else if (textureKey == "metallic") {
                        material->metallic = pTexture;
                    } else if (textureKey == "roughness") {
                        material->roughness = pTexture;
                    } else if (textureKey == "ambientOcclusion") {
                        material->ambientOcclusion = pTexture;
                    } else if (textureKey == "height") {
                        material->height = pTexture;
                    } else if (textureKey == "specular") {
                        material->specular = pTexture;
                    } else if (textureKey == "emissive") {
                        material->emissive = pTexture;
                    }
            }
        }

        material->Triplanar = data["triplanar"] ? data["triplanar"].as<bool>() : false;
        material->TriplanarTilingScale = data["triplanarTilingScale"] ? data["triplanarTilingScale"].as<float>() : 1.0f;

        return true;
    }

    /// <summary>
    /// Deserialize data to currently set material
    /// </summary>
    /// <param name="out"></param>
    /// <param name="pmaterial">material to serialize</param>
    /// <returns></returns>
     void MaterialSerializer::SerializeForModel(YAML::Emitter &out, Material *pMaterial) {
        SetMaterial(pMaterial);
        out << YAML::BeginMap;
        out << YAML::Key << "materialName";
        out << YAML::Value << material->name;
        out << YAML::Key << "uuid";
        out << YAML::Value << material->UUID;
        out << YAML::Key << "materialType";
        out << YAML::Value << material->GetMaterialType();

        out << YAML::Key << "Textures";
        out << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "diffuse";
        out << YAML::Value << *material->diffuse;
        out << YAML::Key << "normal";
        out << YAML::Value << *material->normal;
        out << YAML::Key << "metallic";
        out << YAML::Value << *material->metallic;
        out << YAML::Key << "roughness";
        out << YAML::Value << *material->roughness;
        out << YAML::Key << "ambientOcclusion";
        out << YAML::Value << *material->ambientOcclusion;
        out << YAML::Key << "height";
        out << YAML::Value << *material->height;
        out << YAML::Key << "specular";
        out << YAML::Value << *material->specular;
        out << YAML::Key << "emissive";
        out << YAML::Value << *material->emissive;
        out << YAML::EndMap;
        
        out << YAML::Key << "triplanar";
        out << YAML::Value << material->Triplanar;
        out << YAML::Key << "triplanarTilingScale";
        out << YAML::Value << material->TriplanarTilingScale;


        out << YAML::EndMap;
    }

    /// <summary>
    /// Deserialize data to currently set material
    /// </summary>
    /// <param name="filepath"></param>
    /// <returns></returns>
    bool MaterialSerializer::DeserializeForModel(const YAML::Node &node, Material *material) {
        SetMaterial(material);

        material->name = node["materialName"].as<std::string>();
        material->UUID = node["uuid"].as<std::string>();
        material->SetMaterialType(node["materialType"].as<int>());

        auto textures = node["Textures"];
        if (textures && textures.IsMap()) {
            for (const auto &pair: textures) {
                std::string textureKey = pair.first.as<std::string>();
                Texture texture = pair.second.as<Texture>();
                std::shared_ptr<Texture> pTexture = ResourceManager::GetInstance().LoadTextureFromFile(texture.path);
                if (textureKey == "diffuse") {
                    material->diffuse = pTexture;
                } else if (textureKey == "normal") {
                    material->normal = pTexture;
                } else if (textureKey == "metallic") {
                    material->metallic = pTexture;
                } else if (textureKey == "roughness") {
                    material->roughness = pTexture;
                } else if (textureKey == "ambientOcclusion") {
                    material->ambientOcclusion = pTexture;
                } else if (textureKey == "height") {
                    material->height = pTexture;
                } else if (textureKey == "specular") {
                    material->specular = pTexture;
                } else if (textureKey == "emissive") {
                    material->emissive = pTexture;
                }
            }
        }

        material->Triplanar = node["triplanar"] ? node["triplanar"].as<bool>() : false;
        material->TriplanarTilingScale = node["triplanarTilingScale"] ? node["triplanarTilingScale"].as<float>() : 1.0f;


        return true;
    }
} // namespace Serialize
