//
// Created by Hubert Klonowski on 19/03/2025.
//

#include "Lua_Camera.h"

#include "Components/Camera/Camera.h"
#include "Components/LuaComponent/LuaComponent.h"
#include "GameObject/GameObject.h"
#include <dBrender/dBrender.h>
#include <dBengine/EngineDebug/EngineDebug.h>
#include <Components/ThirdPersonCamera/ThirdPersonCamera.h>

Lua_Camera::Lua_Camera(LuaComponent *lua_component) : Lua(lua_component) { 
        this->uuid = UUID::generateUUID();
        Register(); 
}

void Lua_Camera::Register() {
    lua_component->L.new_usertype<Lua_Camera>("Camera",
                                              // functions
                                              "GetFront", &Lua_Camera::Lua_GetCameraFront, "GetRight",
                                              &Lua_Camera::Lua_GetCameraRight, "GetPitch",
                                              &Lua_Camera::Lua_GetPitchAngle, "GetYaw", &Lua_Camera::Lua_GetYawAngle,
                                              "UpdateCamera", &Lua_Camera::Lua_UpdateCamera, "IsUsed", &Lua_Camera::Lua_IsUsed
    );
    lua_component->L["Camera"] = this;
}

sol::table Lua_Camera::Lua_GetCameraFront() {
    Camera *camera_component = lua_component->gameObject->GetComponent<Camera>();
    sol::table table = lua_component->L.create_table();
    if (camera_component && camera_component->isUsed) {
        table["x"] = camera_component->Front.x;
        table["y"] = camera_component->Front.y;
        table["z"] = camera_component->Front.z;
    }
    return table;
}

sol::table Lua_Camera::Lua_GetCameraRight() {
    Camera *camera_component = lua_component->gameObject->GetComponent<Camera>();
    sol::table table = lua_component->L.create_table();
    if (camera_component && camera_component->isUsed) {
        table["x"] = camera_component->Right.x;
        table["y"] = camera_component->Right.y;
        table["z"] = camera_component->Right.z;
    }
    return table;
}

float Lua_Camera::Lua_GetPitchAngle() {
    ThirdPersonCamera *tppCamera = lua_component->gameObject->GetComponent<ThirdPersonCamera>();
    Camera *camera_component = lua_component->gameObject->GetComponent<Camera>();
    if (tppCamera) {
        return tppCamera->orbitPitch;
    }
    return camera_component ? camera_component->Pitch : 0.0f;
}

float Lua_Camera::Lua_GetYawAngle() {
    ThirdPersonCamera *tppCamera = lua_component->gameObject->GetComponent<ThirdPersonCamera>();
    Camera *camera_component = lua_component->gameObject->GetComponent<Camera>();
    if (tppCamera) {
        return tppCamera->orbitYaw;
    }
    return camera_component ? camera_component->Yaw : 0.0f;
}

void Lua_Camera::Lua_UpdateCamera(float yaw, float pitch) {
    Camera *camera_component = lua_component->gameObject->GetComponent<Camera>();
    if (camera_component && camera_component->isUsed) {
        camera_component->Yaw = yaw;
        camera_component->Pitch = pitch;
        camera_component->UpdateCameraVectors();
    }
}

bool Lua_Camera::Lua_IsUsed(){
    Camera *camera_component = lua_component->gameObject->GetComponent<Camera>();
    return camera_component ? camera_component->isUsed & !camera_component->isLerping : false;
}
