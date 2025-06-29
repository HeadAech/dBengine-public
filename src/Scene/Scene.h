//
// Created by Hubert Klonowski on 14/03/2025.
//

#ifndef SCENE_H
#define SCENE_H
#include <vector>

#include "GameObject/GameObject.h"
#include "Components/WorldEnvironment/WorldEnvironment.h"
#include <memory>
#include <filesystem>

class Scene {
public:
    bool isActive = false;
    /// <summary>
    /// Universal Unique Identifier of the scene.
    /// </summary>
    std::string uuid;

    /// <summary>
    /// Name of the scene.
    /// </summary>
    std::string name;
    std::unique_ptr<GameObject> sceneCameraObject = nullptr;
    std::unique_ptr<GameObject> sceneRootObject;

    std::string Path;

    Scene(std::string name);
    ~Scene();

    /// <summary>
    /// Reassign signals from old to newly loaded scene.
    /// </summary>
    /// <param name="uuid"></param>
    void ReassignSignals(std::string uuid);

    /// <summary>
    /// Creates a new GameObject with the specified name and returns a pointer to it.
    /// It works by submiting the GameObject to a pending list, which will be committed later.
    /// </summary>
    /// <param name="name">Name of the new GameObject</param>
    /// <returns></returns>
    GameObject* CreateGameObject(const std::string& name);
    
    /// <summary>
    /// Checks if name of a gameobject has been already used, if yes, change it.
    /// </summary>
    /// <param name="name"></param>
    /// <returns></returns>
    std::string CheckGameObjectName(const std::string &name);

    /// <summary>
    /// Checks if name of a gameobject has been already used, if yes, change it. [ONLY IN SIBLINGS SCOPE]
    /// </summary>
    /// <param name="name"></param>
    /// <returns></returns>
    std::string CheckGameObjectNameSiblings(const std::string &name, GameObject* parent);

    /// <summary>
    /// Commits all pending GameObjects to the main gameObjects vector.
    /// </summary>
    void CommitPendingGameObjects();

    /// <summary>
    /// Updates all GameObjects in the scene.
    /// </summary>
    /// <param name="deltaTime"></param>
    void Update(float deltaTime) const;

    /// <summary>
    /// <para>DEPRECATED: dBrender handles all the rendering as of version 0.0.3</para>
    /// Renders all GameObjects in the scene.
    /// </summary>
    void Render() const;

    /// <summary>
    /// Recursevily searches for a GameObject with the specified name in the provided vector of GameObjects.
    /// </summary>
    /// <param name="name">Name of searched GameObject</param>
    /// <param name="objects">Source of searching</param>
    /// <returns></returns>
    GameObject *Find(const std::string &name, const std::vector<std::unique_ptr<GameObject>> &objects);

    /// <summary>
    /// Recursively searches for a GameObject with the specified UUID in the provided vector of GameObjects.
    /// </summary>
    /// <param name="uuid">UUID of searched GameObject</param>
    /// <param name="objects">Source of searching</param>
    /// <returns></returns>
    GameObject *FindByUUID(const std::string &uuid, const std::vector<std::unique_ptr<GameObject>> &objects);

    /// <summary>
    /// Returns a GameObject with the specified name.
    /// </summary>
    /// <param name="name">Name of the GameObject to return</param>
    /// <returns></returns>
    GameObject *GetGameObject(const std::string &name);

    /// <summary>
    /// Returns a GameObject with the specified UUID.
    /// </summary>
    /// <param name="uuid">UUID of the GameObject to return</param>
    /// <returns></returns>
    GameObject *GetGameObjectUUID(const std::string &uuid);

    /// <summary>
    /// Safely deletes a GameObject with the specified UUID.
    /// <para>If the GameObject was associated with lights, WE, etc. all resources will be freed first.</para>
    /// </summary>
    /// <param name="uuid"></param>
    void DeleteGameObject(const std::string& uuid);

    
    /// <summary>
    /// Returns a reference to a vector of all GameObjects in the scene.
    /// </summary>
    /// <returns></returns>
    std::vector<GameObject*> GetGameObjects();

    /// <summary>
    /// Returns a pointer to the directional light GameObject in the scene.
    /// </summary>
    /// <returns></returns>
    GameObject *GetDirectionalLight() const;

    /// <summary>
    /// Returns a reference to a vector of all spot lights in the scene.
    /// </summary>
    /// <returns></returns>
    const std::vector<GameObject *> &GetSpotLights() const;

    /// <summary>
    /// Returns a reference to a vector of all point lights in the scene.
    /// </summary>
    /// <returns></returns>
    const std::vector<GameObject *> &GetPointLights() const;

    /// <summary>
    /// Returns a pointer to the WorldEnvironment component in the scene.
    /// </summary>
    /// <returns></returns>
    WorldEnvironment *GetWorldEnvironment() const;

    /// <summary>
    /// Checks if a MeshInstance with the specified model path is in use in the scene.
    /// </summary>
    /// <param name="path">Path to the model</param>
    /// <returns></returns>
    bool MeshInstanceOfPathInUse(const std::string &path) const;

    /// <summary>
    /// Get modify time of a scene
    /// </summary>
    /// <returns></returns>
    std::filesystem::file_time_type GetModifyTime();
    
    /// <summary>
    /// Set Modify time of a scene
    /// </summary>
    /// <param name="modifyTime"></param>
    void SetModifyTime(std::filesystem::file_time_type modifyTime);

    /// <summary>
    /// transfer vectors of lights etc from a loaded scene [just add them to current scene]
    /// </summary>
    /// <param name="scene"></param>
    void transferVectorsFromScene(Scene *scene);

    GameObject *m_DirLight = nullptr;
    std::vector<GameObject *> m_SpotLights;
    std::vector<GameObject *> m_PointLights;
    WorldEnvironment *m_WorldEnvironment = nullptr;

    private:

        bool searchMeshInstanceInUseInChildren(GameObject *gameObject, const std::string &path) const;
        std::filesystem::file_time_type modifyTime;
        std::vector<std::unique_ptr<GameObject>> m_PendingGameObjects;

        friend class SceneSerializer;
};



#endif //SCENE_H
