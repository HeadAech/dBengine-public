//
// Created by Hubert Klonowski on 14/03/2025.
//
#pragma once
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <string>
#include <vector>
#include <memory>
#include "Component/Component.h"
#include "Components/Transform/Transform.h"
#include "BoundingVolume/AABB.h"
#include "Components/Lights/DirectionalLight/DirectionalLight.h"
#include "Components/Lights/SpotLight/SpotLight.h"
#include "Components/Lights/PointLight/PointLight.h"
#include "Components/WorldEnvironment/WorldEnvironment.h"
#include <Signal/Signals.h>


class Component;

class GameObject {

    void forceUpdate(float deltaTime);
    
public:
    
    /// <summary>
    /// Forces the GameObject to update its and parent's transforms.
    /// </summary>
    void ForceUpdateTransform();
    
    /// <summary>
    /// Universal Unique Identifier of the GameObject.
    /// </summary>
    std::string uuid;

    /// <summary>
    /// Name of the GameObject.
    /// </summary>
    std::string name;

    /// <summary>
    /// Only if it is a scene loaded to object it has it filled.
    /// </summary>
    std::string pathToScene;
    
    /// Every GameObject has a Transform component that stores its position, rotation, and scale.
    /// </summary>
    Transform transform;

    /// <summary>
    /// Pointer to the parent.
    /// </summary>
    GameObject* parent = nullptr;

    /// <summary>
    /// Vector of child GameObjects.
    /// </summary>
    std::vector<std::unique_ptr<GameObject>> children;

    /// <summary>
    /// Vector of components attached to the GameObject.
    /// </summary>
    std::vector<std::unique_ptr<Component>> components;

    /// <summary>
    /// Is the GameObject enabled
    /// </summary>
    bool m_enabled = true;
    bool isScene = false; 

    GameObject(const std::string& name);
    ~GameObject();

    /// <summary>
    /// Updates the GameObject and its components.
    /// </summary>
    /// <param name="deltaTime">Time since last frame</param>
    void Update(float deltaTime);

    /// <summary>
    /// <para>DEPRECATED: dBrender handles all the rendering as of version 0.0.3</para>
    /// Renders itself and its children.
    /// </summary>
    void Render();

    /// <summary>
    /// Appends a child GameObject to the children vector.
    /// </summary>
    /// <param name="child">Pointer to the child (GameObject)</param>
    void AddChild(GameObject* child);

    /// <summary>
    /// Appends a child GameObject to the children vector.
    /// </summary>
    /// <param name="child">A Unique Pointer to the child</param>
    void AddChild(std::unique_ptr<GameObject> child);

    /// <summary>
    /// Moves the GameObject to a new parent.
    /// </summary>
    /// <param name="newParent">Target parent</param>
    void Reparent(GameObject* newParent);
    void childrenEnableDisable(bool m_enabled);

    /// <summary>
    /// Returns a matrix that represents the world transform of the GameObject.
    /// </summary>
    /// <returns>World matrix (glm::mat4)</returns>
    glm::mat4 GetWorldTransform();

    /// <summary>
    /// Returns the UUID of the GameObject.
    /// </summary>
    /// <returns>UUID (string)</returns>
    std::string GetUUID();

    /// <summary>
    /// Adds a component of type T to the GameObject.
    /// </summary>
    /// <typeparam name="T">Type of the component</typeparam>
    /// <typeparam name="...Args"></typeparam>
    /// <param name="...args"></param>
    /// <returns>Pointer to a new component</returns>
    template <typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        auto component = std::make_unique<T>(std::forward<Args>(args)...);
        component->gameObject = this;

        components.push_back(std::move(component));

        if (DirectionalLight *dirLight = dynamic_cast<DirectionalLight *>(components.back().get())) {
            Signals::Scene_AddedDirectionalLight.emit(uuid);
        } else if (PointLight *pointLight = dynamic_cast<PointLight *>(components.back().get())) {
            Signals::Scene_AddedPointLight.emit(uuid);
        } else if (SpotLight *spotLight = dynamic_cast<SpotLight *>(components.back().get())) {
            Signals::Scene_AddedSpotLight.emit(uuid);
        } else if (WorldEnvironment *worldEnv = dynamic_cast<WorldEnvironment *>(components.back().get())) {
            Signals::Scene_AddedWorldEnvironment.emit(uuid);
        }

        return static_cast<T *>(components.back().get());
    }

    /// <summary>
    /// Searches for a component of type T in the components attached to the GameObject.
    /// </summary>
    /// <typeparam name="T">Type of component</typeparam>
    /// <returns>Pointer to component</returns>
    template <typename T>
    T* GetComponent() {
        for (auto& component : components) {
            if (T* casted = dynamic_cast<T*>(component.get())) {
                return casted;
            }
        }
        return nullptr; // Not found
    }

    void RemoveComponent(Component* component);

    GameObject* GetChild(const std::string& name);
    GameObject* GetChildUUID(const std::string& uuid);

    /// <summary>
    /// Frees GameObject and all of its resources.
    /// <para>WARN: Not used right now.</para>
    /// </summary>
    void Die();

    /// <summary>
    /// Disables the GameObject, making it inactive.
    /// </summary>
    void Disable();

    /// <summary>
    /// Enables the GameObject, making it active.
    /// </summary>
    void Enable();
};



#endif //GAMEOBJECT_H
