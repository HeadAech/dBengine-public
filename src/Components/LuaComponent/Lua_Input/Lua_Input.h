//
// Created by Hubert Klonowski on 19/03/2025.
//

#ifndef LUA_INPUT_H
#define LUA_INPUT_H
#include "Components/LuaComponent/Lua.h"

class Lua_Input : public Lua {
public:
    Lua_Input(){};
    ~Lua_Input();
    Lua_Input(LuaComponent *lua_component);
    void Register() override;

    void Lua_MouseMotion(float x, float y);

    // setters
    void Lua_SetCursorLocked(bool mode);

    // getters
    sol::table Lua_GetMousePosition();
    bool Lua_IsCursorLocked();

    // inputs actions
    bool Lua_IsActionJustPressed(std::string actionName);
    bool Lua_IsActionJustReleased(std::string actionName);
    bool Lua_IsActionPressed(std::string actionName);
};


#endif // LUA_INPUT_H
