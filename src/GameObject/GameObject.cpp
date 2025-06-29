//
// Created by Hubert Klonowski on 14/03/2025.
//

#include "GameObject.h"

#include <iostream>
#include <ostream>

#include "Components/Camera/Camera.h"
#include "Components/MeshInstance/MeshInstance.h"
#include "dBrender/dBrender.h"
#include "Helpers/UUID/UUID.h"
#include "dBengine/dBengine.h"

GameObject::GameObject(const std::string &name) : name(name) { 
    this->uuid = UUID::generateUUID(); 
}

GameObject::~GameObject() {
}

GameObject* findByUUID(const std::string& uuid, const std::vector<std::unique_ptr<GameObject>>& objects)
{
    for (const auto& object : objects)
    {
        if (object->GetUUID() == uuid)
        {
            return object.get();
        }

        // Recurse into children
        if (!object->children.empty())
        {
            GameObject* found = findByUUID(uuid, object->children);
            if (found)
                return found;
        }
    }
    return nullptr;
}

GameObject* findByName(const std::string& name, const std::vector<std::unique_ptr<GameObject>>& objects)
{
    for (const auto& object : objects)
    {
        if (object->name == name)
        {
            return object.get();
        }

        // Recurse into children
        if (!object->children.empty())
        {
            GameObject* found = findByUUID(name, object->children);
            if (found)
                return found;
        }
    }
    return nullptr;
}

void GameObject::RemoveComponent(Component* component)
{
    auto it = std::remove_if(components.begin(), components.end(),
        [component](const std::unique_ptr<Component>& comp)
        {
            return comp->GetUUID() == component->GetUUID();
        });

    components.erase(it, components.end());
}

GameObject* GameObject::GetChildUUID(const std::string& uuid)
{
    return findByUUID(uuid, this->children);
}

GameObject* GameObject::GetChild(const std::string& name)
{
    return findByName(name, this->children);
}

void GameObject::Die() {
    std::cout << "Killing GameObject " << name << std::endl;
    // this->~GameObject();
}

void GameObject::Disable() { 
    this->m_enabled = false; 
    //childrenEnableDisable(this->m_enabled); -> Later usage for hide all or something like that

    for (int i = 0; i < children.size(); i++)
    {
        children[i].get()->Disable();
    }
    ForceUpdateTransform();

}
void GameObject::Enable() { 
    this->m_enabled = true; 
    //childrenEnableDisable(this->m_enabled); -> Later usage for hide all or something like that
    for (int i = 0; i < children.size(); i++)
    {
        children[i].get()->Enable();
    }
    ForceUpdateTransform();

}



void GameObject::Update(float deltaTime) {
    // if (name == "Cube") {
    //     glm::vec3 rot = transform.GetEulerRotation();
    //     // std::cout << deltaTime * 40 << std::endl;
    //     this->transform.SetEulerRotation({rot.x, rot.y + deltaTime * 40, rot.z + deltaTime * 40});
    // }
    if (transform.IsDirty()) {
        forceUpdate(deltaTime);
        return;
    }

    if (!m_enabled)
        return;

    if (EngineSettings::GetEngineSettings().m_inPlayMode) {

        for (auto &component: components) {
            component->Update(deltaTime);
        }
    }

    for (int i = 0; i < children.size(); i++ ) {
        auto& child = children[i];
        if (child)
        {
            child->Update(deltaTime);
        }
    }
}

void GameObject::forceUpdate(float deltaTime) {
    ForceUpdateTransform();

    for (int i = 0; i < components.size(); i++)
    {
        auto &component = components[i];

        if (EngineSettings::GetEngineSettings().m_inPlayMode)
            component->Update(deltaTime);
        if (Camera* camera = dynamic_cast<Camera*>(component.get())) {
            camera->UpdateCameraVectors();
        }

        if (auto light = dynamic_cast<PointLight*>(component.get()))
        {
            light->propertyWatch.Position = true;
        }

        if (auto light = dynamic_cast<SpotLight*>(component.get()))
        {
            light->propertyWatch.Position = true;
        }
    }

    for (int i = 0; i < children.size(); i++)
    {
        auto &child = children[i];
        if (child)
        {
            child->forceUpdate(deltaTime);

        }
    }
}

void GameObject::ForceUpdateTransform() {
    if (parent) {
        transform.ComputeModelMatrix(parent->transform.GetModelMatrix());
    } else {
        transform.ComputeModelMatrix();
    }
}


void GameObject::Render() {
    if (!m_enabled)
        return;

    for (auto& component: components) {
        component->Render();
    }

    for (auto& child: children) {
        child->Render();
    }
}

void GameObject::AddChild(GameObject* child) {
    children.push_back(std::unique_ptr<GameObject>(child));
    dBrender::GetInstance().m_flatGameObjects.push_back(children.back().get());
    children.back()->parent = this;
}

void GameObject::AddChild(std::unique_ptr<GameObject> child) {
    children.push_back(std::move(child));
    dBrender::GetInstance().m_flatGameObjects.push_back(children.back().get());
    children.back()->parent = this;
}


glm::mat4 GameObject::GetWorldTransform() {
    if (parent) {
        return parent->GetWorldTransform() * transform.GetModelMatrix();
    }
    return transform.GetModelMatrix();
}

std::string GameObject::GetUUID() { return this->uuid; }

void GameObject::Reparent(GameObject *newParent) {
    if (parent) {
        // find and remove current node from original parent's position
        auto& siblings = parent->children;
        auto it = std::find_if(siblings.begin(), siblings.end(),
            [this](const std::unique_ptr<GameObject>& child) {
                return child.get() == this;
            });

        if (it != siblings.end()) {
            // change ownership to new parent
            std::unique_ptr<GameObject> movingNode = std::move(*it);
            siblings.erase(it);

            // Add the node to the new parent's children
            newParent->AddChild(std::move(movingNode));
        }
    } else {
        newParent->AddChild(std::unique_ptr<GameObject>(this));
    }
    this->parent = newParent;
}

void GameObject::childrenEnableDisable(bool m_enabled){
    for(auto& child : this->children){
        child->childrenEnableDisable(m_enabled);
        child->m_enabled = m_enabled;
    }
}
