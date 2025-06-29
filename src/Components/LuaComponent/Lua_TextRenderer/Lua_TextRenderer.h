//
// Created by Hubert Klonowski on 02/04/2025.
//

#ifndef LUA_TEXTRENDERER_H
#define LUA_TEXTRENDERER_H
#include "Components/LuaComponent/Lua.h"


class Lua_TextRenderer : public Lua {
public:
    Lua_TextRenderer(){};
    ~Lua_TextRenderer() = default;
    Lua_TextRenderer(LuaComponent *lua_component);
    void Register() override;


    void Lua_SetText(std::string text);
};

#endif // LUA_TEXTRENDERER_H
