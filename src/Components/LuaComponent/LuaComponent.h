//
// Created by Hubert Klonowski on 15/03/2025.
//

#ifndef LUACOMPONENT_H
#define LUACOMPONENT_H
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <ostream>

#include "Component/Component.h"
#include "InputManager/Input.h"
#include "Lua_Camera/Lua_Camera.h"
#include "Lua_GameObject/Lua_GameObject.h"
#include "Lua_Input/Lua_Input.h"
#include "Lua_TextRenderer/Lua_TextRenderer.h"
#include "sol.hpp"

struct vec3 {
    float x;
    float y;
    float z;
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
};

class GameObject;

struct ExposedVar
{
    std::string name;
    sol::object defaultValue;
};

class LuaComponent : public Component {
    // TODO: Implement functions to manipulate gameobject's state

    // Setters

    void Lua_UpdateCameraVectors();


    // Getters
    // Input

    void Register();

    void callOnReady();
    bool calledOnReady = false;

    std::vector<ExposedVar> m_ExposedVars;
    std::vector<std::string> m_SignalNames;
    void bindExport();

    void prepareTypes();

    bool onReadyErr = false;
    bool onMouseMotionErr = false;
    bool onUpdateErr = false;
    bool disconnectSignalsErr = false;
    bool connectSignalsErr = false;

public:
    // lua_State* L;
    sol::state L;
    std::string scriptPath;
    std::string scriptName;

    LuaComponent();
    LuaComponent(const std::string &scriptPath);
    ~LuaComponent();

    std::vector<ExposedVar>& GetExposedVars();

    std::vector<std::unique_ptr<Lua>> luaComponentContainer;

    void SetScript(const std::string& path);
    void RemoveScript();

    /* Since we do not use each one i will just put them to vector.
    std::unique_ptr<Lua_GameObject> lua_game_object;
    std::unique_ptr<Lua_Input> lua_input;
    std::unique_ptr<Lua_Camera> lua_camera;
    std::unique_ptr<Lua_TextRenderer> lua_textRenderer;
    */

    void ReloadScript();

    void Update(float deltaTime) override;

    void Lua_Quit();
};


#endif // LUACOMPONENT_H
