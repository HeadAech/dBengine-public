//
// Created by Hubert Klonowski on 14/03/2025.
//

#ifndef COMPONENT_H
#define COMPONENT_H
#include <string>
#include <Helpers/UUID/UUID.h>
#include <dBengine/EngineDebug/EngineDebug.h>
#include <Helpers/fonts/IconsFontAwesome4.h>


class GameObject;

class Component {
    
    

    friend class SceneSerializer;

    public:

    bool enabled = true;
    std::string uuid;

    std::string name = "Component";
    std::string icon = ICON_FA_CUBE;

    GameObject* gameObject = nullptr;

    Component();
    virtual ~Component() = default;
    virtual void Update(float deltaTime);
    virtual void Render();

    virtual void Enable();
    virtual void Disable();

    virtual void Die();
    std::string GetUUID();
    

};



#endif //COMPONENT_H
