//
// Created by Hubert Klonowski on 14/03/2025.
//

#include "MeshInstance.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "GameObject/GameObject.h"
#include <assimp/config.h>
#include <ResourceManager/ResourceManager.h>
#include <Helpers/TimerHelper/TimerHelper.h>

MeshInstance::MeshInstance() {
    name = "MeshInstance";
    icon = ICON_FA_CUBE;
}

MeshInstance::~MeshInstance() { 

}



void MeshInstance::Render() {
    if (!enabled) return;
    if (!shader) return;
}

void MeshInstance::LoadModel(const std::string &path) {
    m_modelPath = path;
    model = ResourceManager::GetInstance().LoadMeshFromFile(path);
    MaterialOverride = nullptr;
}


void MeshInstance::SetShader(Shader *shader) {
    this->shader = shader;
}

void MeshInstance::InstantiateMesh(std::string pathToModel) {
    //dBrender::GetInstance().AddGameObject(pathToModel, gameObject);
}

void MeshInstance::Disable() {
    Component::Disable();
    gameObject->ForceUpdateTransform();
    //if (m_instanced)
        //dBrender::GetInstance().OnGameObjectTransformChanged(gameObject);
}

void MeshInstance::Enable() {
    Component::Enable();
    gameObject->ForceUpdateTransform();
   // if (m_instanced)=
//        dBrender::GetInstance().OnGameObjectTransformChanged(gameObject);*/
}

std::map<std::string, BoneInfo> &MeshInstance::GetBoneInfoMap() { return model->BoneInfoMap; }

int &MeshInstance::GetBoneCount() { return model->m_BoneCounter; }
