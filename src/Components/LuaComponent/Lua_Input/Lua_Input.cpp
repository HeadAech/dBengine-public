//
// Created by Hubert Klonowski on 19/03/2025.
//

#include "Lua_Input.h"

#include "Components/LuaComponent/LuaComponent.h"
#include "Signal/Signals.h"

Lua_Input::Lua_Input(LuaComponent *lua_component) : Lua(lua_component) {
    Register();
    this->uuid = UUID::generateUUID();
    Signals::CursorOffsetChanged.connect(this->uuid, [this](float x, float y) { Lua_MouseMotion(x, y); });
}

Lua_Input::~Lua_Input() { 
    Signals::CursorOffsetChanged.disconnect(this->uuid); 
}

void Lua_Input::Register() {

    lua_component->L.new_usertype<Lua_Input>(
            "Input",
            // mouse
            "GetMousePosition", &Lua_Input::Lua_GetMousePosition, "IsCursorLocked", &Lua_Input::Lua_IsCursorLocked,
            "SetCursorLocked", &Lua_Input::Lua_SetCursorLocked,

            // actions
            "IsActionJustPressed", &Lua_Input::Lua_IsActionJustPressed, "IsActionJustReleased",
            &Lua_Input::Lua_IsActionJustReleased, "IsActionPressed", &Lua_Input::Lua_IsActionPressed);

    lua_component->L["Input"] = this;
}


void Lua_Input::Lua_MouseMotion(float x, float y) {
    if (!lua_component->enabled)
        return;

    auto onMouseMotion = lua_component->L["onMouseMotion"];
    if (onMouseMotion.valid() && onMouseMotion.get_type() == sol::type::function) {
        try {
            onMouseMotion(x, y);
        } catch (const sol::error &e) {
            std::cerr << "Lua error calling onMouseMotion: " << e.what() << std::endl;
        }
    }
}

sol::table Lua_Input::Lua_GetMousePosition() {
    glm::vec2 mousePos = Input::GetInstance().GetMousePosition();
    sol::table table = lua_component->L.create_table();
    table["x"] = mousePos.x;
    table["y"] = mousePos.y;
    return table;
}

bool Lua_Input::Lua_IsCursorLocked() { return Input::GetInstance().m_cursorLocked; }

bool Lua_Input::Lua_IsActionJustPressed(std::string actionName) {
    return Input::GetInstance().IsActionJustPressed(actionName);
}

bool Lua_Input::Lua_IsActionPressed(std::string actionName) { return Input::GetInstance().IsActionPressed(actionName); }

bool Lua_Input::Lua_IsActionJustReleased(std::string actionName) {
    return Input::GetInstance().IsActionJustReleased(actionName);
}

void Lua_Input::Lua_SetCursorLocked(bool mode) { Input::GetInstance().SetCursorLocked(mode); }
