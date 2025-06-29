//
// Created by Hubert Klonowski on 02/04/2025.
//

#include "Lua_TextRenderer.h"

#include "Components/LuaComponent/LuaComponent.h"
#include "Components/TextRenderer/TextRenderer.h"
#include "GameObject/GameObject.h"

Lua_TextRenderer::Lua_TextRenderer(LuaComponent *lua_component) : Lua(lua_component) {
    this->uuid = UUID::generateUUID();
    Register();
}

void Lua_TextRenderer::Register() {
    lua_component->L.new_usertype<Lua_TextRenderer>("TextRenderer", "SetText", &Lua_TextRenderer::Lua_SetText);

    lua_component->L["TextRenderer"] = this;
}

void Lua_TextRenderer::Lua_SetText(std::string text) {
    if (auto textRenderer = lua_component->gameObject->GetComponent<TextRenderer>()) {
        textRenderer->text = text;
    }
}
