//
// Created by Hubert Klonowski on 19/03/2025.
//

#ifndef LUA_CAMERA_H
#define LUA_CAMERA_H
#include "Components/LuaComponent/Lua.h"


class Lua_Camera : public Lua {

public:
    Lua_Camera(){};
    Lua_Camera(LuaComponent *lua_component);
    ~Lua_Camera() = default;
    void Register() override;

    sol::table Lua_GetCameraFront();
    sol::table Lua_GetCameraRight();
    float Lua_GetPitchAngle();
    float Lua_GetYawAngle();
    void Lua_UpdateCamera(float yaw, float pitch);
    bool Lua_IsUsed();

};


#endif // LUA_CAMERA_H
