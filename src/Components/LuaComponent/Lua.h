//
// Created by Hubert Klonowski on 16/03/2025.
//

#ifndef LUA_H
#define LUA_H
#include <sol.hpp>
#include <GameObject/GameObject.h>

class LuaComponent;


class Lua {
 

public:
    std::string uuid;
    Lua() : lua_component() {};
    virtual ~Lua() = default;
    LuaComponent *lua_component;
    Lua(LuaComponent *lua_component) : lua_component(lua_component) {}
    virtual void Register(){};
};

#endif // LUA_H
