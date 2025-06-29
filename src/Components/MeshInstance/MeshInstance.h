//
// Created by Hubert Klonowski on 14/03/2025.
//

#ifndef MESHINSTANCE_H
#define MESHINSTANCE_H
#include <map>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "assimp/scene.h"
#include "Component/Component.h"
#include "glad/glad.h"
#include "imgui_impl/imgui_impl_opengl3_loader.h"
#include "Shader/Shader.h"
#include "stb_image.h"
#include "dBrender/dBrender.h"
#include <Material/Material.h>
#include "Mesh.h"
#include <Components/Animator/Bone/Bone.h>
#include "Model.h"


class MeshInstance : public Component {

    Shader* shader = nullptr;


public:
    std::shared_ptr<Model> model;

    std::string directory;
    std::string m_modelPath;

    std::shared_ptr<Material> MaterialOverride = nullptr;

    MeshInstance();
    ~MeshInstance();

    void Render() override;

    void LoadModel(const std::string& path);


    void SetShader(Shader* shader);

    void InstantiateMesh(std::string pathToModel);


    void Disable() override;
    void Enable() override;


    bool m_useRefraction = false;
    void useRefraction(bool value) { m_useRefraction = value; }
    bool isUsingRefraction() const { return m_useRefraction; }

    bool m_useReflection = false;
    void useReflection(bool value) { m_useReflection = value; }
    bool isUsingReflection() const { return m_useReflection; }

    bool m_useVolumetric = false;
    void useVolumetric(bool value) {
        m_useVolumetric = value;
    }
    bool isUsingVolumetric() const { return m_useVolumetric; }

    std::map<std::string, BoneInfo> &GetBoneInfoMap();
    int &GetBoneCount();


    float density = 0.1f;
    int samples = 128;
    glm::vec3 fogColor = glm::vec3(0.4f, 0.4f, 1.0f);
    float scattering = 0.60f;
    

};

#endif //MESHINSTANCE_H
