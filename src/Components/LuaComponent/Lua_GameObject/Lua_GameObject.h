//
// Created by Hubert Klonowski on 16/03/2025.
//

#ifndef LUA_GAMEOBJECT_H
#define LUA_GAMEOBJECT_H
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <sol.hpp>
#include "glm/glm.hpp"

#include "Components/LuaComponent/Lua.h"


class LuaComponent;

class Lua_GameObject : public Lua {
public:
    Lua_GameObject(){};
    Lua_GameObject(LuaComponent *lua_component);
    ~Lua_GameObject() = default;
    void Register() override;

    void Test();

    // getters
    sol::table GetLocalPosition();
    sol::table GetEulerRotation();
    sol::table GetQuatRotation();
    sol::table GetScale();
    std::string GetUUID();

    // setters
    void SetLocalPosition(float x, float y, float z);
    void SetEulerRotation(float x, float y, float z);
    void SetQuatRotation(sol::table q1);
    void SetScale(float x, float y, float z);

    // Camera stuff
    void RotateCamera(sol::table eulerRot);

    // Quat related stuff
    sol::table MultiplyQuatsTables(sol::table q1, sol::table q2);
    sol::table Slerp(sol::table q1, sol::table q2, float weight);
};


#endif // LUA_GAMEOBJECT_H
