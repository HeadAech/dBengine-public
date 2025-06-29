//
// Created by Hubert Klonowski on 14/03/2025.
//

#include "Scene.h"
#include <dBengine/EngineDebug/EngineDebug.h>
#include <regex>
#include <string>
#include "Helpers/Util.h"
#include <Signal/Signals.h>
#include <dBrender/dBrender.h>
#include "Components/MeshInstance/MeshInstance.h"
#include <ResourceManager/ResourceManager.h>

static std::mutex s_SceneMutex;

Scene::Scene(std::string name) {
    this->name = name;
    this->uuid = UUID::generateUUID();
    this->isActive = true;
    sceneRootObject = std::make_unique<GameObject>(name);
    sceneRootObject->isScene = true;
    ReassignSignals(uuid);

}

void Scene::ReassignSignals(std::string uuid) {
    Signals::Scene_AddedDirectionalLight.disconnect(this->uuid);
    Signals::Scene_AddedPointLight.disconnect(this->uuid);
    Signals::Scene_AddedSpotLight.disconnect(this->uuid);
    Signals::Scene_AddedWorldEnvironment.disconnect(this->uuid);

    this->isActive = true;
    Signals::Scene_AddedDirectionalLight.connect(this->uuid, [this](std::string uuid) {
        auto g = GetGameObjectUUID(uuid);
        if (isActive && g && m_DirLight == nullptr) {
            m_DirLight = g;
        }
    });

    Signals::Scene_AddedPointLight.connect(this->uuid, [this](std::string uuid) {
        auto g = GetGameObjectUUID(uuid);
        if (isActive && g) {
            auto it = std::find_if(m_PointLights.begin(), m_PointLights.end(),
                                   [&g](GameObject *obj) { return obj->GetUUID() == g->GetUUID(); });
            if (it == m_PointLights.end()) {
                m_PointLights.push_back(g);
            }
        }
    });

    Signals::Scene_AddedSpotLight.connect(this->uuid, [this](std::string uuid) {
        auto g = GetGameObjectUUID(uuid);
        if (isActive && g) {
            auto it = std::find_if(m_SpotLights.begin(), m_SpotLights.end(),
                                   [&g](GameObject *obj) { return obj->GetUUID() == g->GetUUID(); });
            if (it == m_SpotLights.end()) {
                m_SpotLights.push_back(g);
            }
        }
    });

    Signals::Scene_AddedWorldEnvironment.connect(this->uuid, [this](std::string uuid) {
        auto g = GetGameObjectUUID(uuid);
        if (isActive && g && m_WorldEnvironment == nullptr) {
                m_WorldEnvironment = g->GetComponent<WorldEnvironment>();
                dBrender &renderer = dBrender::GetInstance();
                m_WorldEnvironment->SetShaders(renderer.m_hdriProcessshader.get(), renderer.m_hdriSkyboxShader.get(),
                                              renderer.m_physicalSkyShader.get(), renderer.m_volumetricShader.get(),
                                              renderer.m_irradianceConvoShader.get());
        }
    });
}

Scene::~Scene() {
    Signals::Scene_AddedDirectionalLight.disconnect(uuid);
    Signals::Scene_AddedPointLight.disconnect(uuid);
    Signals::Scene_AddedSpotLight.disconnect(uuid);
    Signals::Scene_AddedWorldEnvironment.disconnect(uuid);
    
    if (sceneRootObject && isActive) {
        sceneRootObject->children.clear();
    }
    sceneRootObject = nullptr;
}


GameObject* Scene::CreateGameObject(const std::string &name) {
    if (name.empty()) {
        EngineDebug::GetInstance().PrintError("GameObject name cannot be empty!");
        return nullptr;
    }
    
    auto gameObject = std::make_unique<GameObject>(CheckGameObjectNameSiblings(name,sceneRootObject.get()));

    m_PendingGameObjects.push_back(std::move(gameObject));
    return m_PendingGameObjects.back().get();
}

std::string Scene::CheckGameObjectName(const std::string &name) {
    std::string newName = name;
    while (auto go = GetGameObject(newName)) {
        // EngineDebug::GetInstance().PrintWarning("GameObject with name " + name + " already exists!");
        std::string num = Util::GetNumberFromEndOfString(go->name);

        if (!num.empty()) {
            int number = std::stoi(num);
            newName = name.substr(0, newName.size() - num.size()) + std::to_string(number + 1);
        } else {
            newName += "1";
        }
    }
    return newName;


}

std::string Scene::CheckGameObjectNameSiblings(const std::string &name, GameObject *parent){
    std::string newName = name;
    for (int i = 0; i < parent->children.size(); i++){
        if (parent->children.at(i)->name == newName){
            std::string num = Util::GetNumberFromEndOfString(newName);

            if (!num.empty()) {
                int number = std::stoi(num);
                newName = name.substr(0, newName.size() - num.size()) + std::to_string(number + 1);
            } else {
                newName += "1";
            }
        }
    }
    return newName;
};

void Scene::CommitPendingGameObjects()
{
    if (m_PendingGameObjects.empty()) {
        return; // Nothing to commit
    }
    for (auto &pendingObject: m_PendingGameObjects) {
        // Add the pending object to the main gameObjects vector
        sceneRootObject->AddChild(std::move(pendingObject));
    }

    // Clear the pending objects vector after committing
    m_PendingGameObjects.clear();

}

void Scene::Update(float deltaTime) const {
    sceneCameraObject->Update(deltaTime);
    sceneRootObject->Update(deltaTime);
}

void Scene::Render() const {
    sceneRootObject->Render();
}
GameObject *Scene::GetGameObject(const std::string &name) { 
    if (sceneRootObject->name == name){
        return sceneRootObject.get();
    }
    auto found = Find(name, sceneRootObject->children);
    if (!found)
    {
        found = Find(name, m_PendingGameObjects);
    }

    return found;
}

GameObject *Scene::GetGameObjectUUID(const std::string &uuid) { 
    if (sceneRootObject->uuid == uuid) {
        return sceneRootObject.get();
    }
    auto found = FindByUUID(uuid, sceneRootObject->children);
    if (!found) {
        found = FindByUUID(uuid, m_PendingGameObjects);
    }
    return found;
}

void Scene::DeleteGameObject(const std::string &uuid) {

    GameObject *gameObject = GetGameObjectUUID(uuid);

    if (!gameObject) {
        EngineDebug::GetInstance().PrintError("GameObject with UUID " + uuid + " not found!");
        return;
    }

    if (m_DirLight == gameObject) {
        m_DirLight = nullptr;
    } 
    
    // check point lights
    auto it_pointLights = std::remove_if(m_PointLights.begin(), m_PointLights.end(),
                                         [uuid](GameObject *obj) { return obj->GetUUID() == uuid; });

    if (it_pointLights != m_PointLights.end()) {
        m_PointLights.erase(it_pointLights, m_PointLights.end());
    }

    // check spot lights
    auto it_spotLights = std::remove_if(m_SpotLights.begin(), m_SpotLights.end(),
                                        [uuid](GameObject *obj) { return obj->GetUUID() == uuid; });

    if (it_spotLights != m_SpotLights.end()) {
        m_SpotLights.erase(it_spotLights, m_SpotLights.end());
    }

    // check world environment
    if (m_WorldEnvironment) {
        if (m_WorldEnvironment->gameObject == gameObject) {
            m_WorldEnvironment = nullptr;
        }
    }

    // check cameras
    if (gameObject->GetComponent<Camera>() && gameObject->GetComponent<Camera>()->isUsed) {
        Signals::Engine_ReturnToSceneCamera.emit(false);
    }

   
    if (auto meshInstance = gameObject->GetComponent<MeshInstance>()) {
        if (!MeshInstanceOfPathInUse(meshInstance->m_modelPath)) {
            EngineDebug::GetInstance().PrintInfo("Mesh " + meshInstance->m_modelPath + " is not in use, deleting it.");
            //ResourceManager::GetInstance().DeleteMesh(meshInstance->m_modelPath);
        }
        auto &resMng = ResourceManager::GetInstance();
        printf("sel");
    }

    //Do we want to make the same stuff with animations // other resources?

    
    GameObject* parentObject = gameObject->parent;
    if (parentObject) {
        Signals::Render_RemoveGameObject.emit(gameObject);
        auto it = std::find_if(parentObject->children.begin(), parentObject->children.end(),
                               [gameObject](std::unique_ptr<GameObject> &obj) { return obj->uuid == gameObject->uuid; });
        
        int currentChildrenSize = gameObject->children.size();
        for (int i = 0; i < currentChildrenSize; i++) { //we will get all eventually. 
            DeleteGameObject(gameObject->children.at(0)->uuid);
        }
        
        if (it != parentObject->children.end()) {
            parentObject->children.erase(it);
        }

    } 
    else{
        if (sceneRootObject->uuid == gameObject->uuid){
            Signals::NewScene.emit("New Scene");
        }
    }



}

std::vector<GameObject*> Scene::GetGameObjects() {

    std::vector<GameObject *> gameObjects;
    dBrender::GetInstance().m_flatGameObjects;
    return gameObjects;
    //return sceneRootObject->children; No longer children, get EVERYTHING
}


GameObject *Scene::GetDirectionalLight() const { return m_DirLight; }

const std::vector<GameObject *> &Scene::GetSpotLights() const { return m_SpotLights; }

const std::vector<GameObject *> &Scene::GetPointLights() const { return m_PointLights; }

WorldEnvironment *Scene::GetWorldEnvironment() const { return m_WorldEnvironment; }

bool Scene::MeshInstanceOfPathInUse(const std::string &path) const { 
    
    for (const auto &gameObject: sceneRootObject->children) {
        if (searchMeshInstanceInUseInChildren(gameObject.get(), path))
            return true;
    }

    return false;

}

std::filesystem::file_time_type Scene::GetModifyTime() { 
    return modifyTime; 
}

void Scene::SetModifyTime(std::filesystem::file_time_type modifyTime) {
        this->modifyTime = modifyTime; }

void Scene::transferVectorsFromScene(Scene *scene) {
    /* If there is only 1 dir light, ignore it then.
    if (m_DirLight){
        this->m_DirLight = scene->m_DirLight;
    }
    */
    /* If there is only 1 world enviro, ignore it then.
    if (m_WorldEnvironment){
        this->m_WorldEnvironment = scene->m_WorldEnvironment;
    }
    */
    
    for(int i = 0; i < scene->m_PointLights.size(); i++){
        this->m_PointLights.push_back(scene->m_PointLights.at(i));
    }

    for (int i = 0; i < scene->m_SpotLights.size(); i++) {
        this->m_SpotLights.push_back(scene->m_SpotLights.at(i));
    }

}

bool Scene::searchMeshInstanceInUseInChildren(GameObject *gameObject, const std::string &path) const {

    auto component = gameObject->GetComponent<MeshInstance>();
    if (auto *meshInstance = dynamic_cast<MeshInstance *>(component)) {
        if (meshInstance->m_modelPath == path) {
            return true;
        }
    }

    if (gameObject->children.size() > 0) {
        for (const auto &child: gameObject->children) {
            return searchMeshInstanceInUseInChildren(child.get(), path);
        }
    }

}

GameObject *Scene::Find(const std::string &name, const std::vector<std::unique_ptr<GameObject>> &objects) {
    for (const auto &object: objects) {
        if (object->name == name) {
            return object.get();
        }

        // Recurse into children
        if (!object->children.empty()) {
            GameObject *found = Find(name, object->children);
            if (found)
                return found;
        }
    }
    return nullptr;
}

GameObject *Scene::FindByUUID(const std::string &uuid, const std::vector<std::unique_ptr<GameObject>> &objects){
    for (const auto &object: objects) {
        if (object->GetUUID() == uuid) {
            return object.get();
        }

        // Recurse into children
        if (!object->children.empty()) {
            GameObject *found = FindByUUID(uuid, object->children);
            if (found)
                return found;
        }
    }
    return nullptr;
}
