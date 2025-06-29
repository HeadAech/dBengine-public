#include "LuaComponent.h"

#include <filesystem>

#include "GameObject/GameObject.h"
#include "Signal/Signals.h"
#include "dBengine/dBengine.h"
#include "Lua_Animator/Lua_Animator.h"
#include "Lua_ParticleSystem/Lua_ParticleSystem.h"
#include <Components/Control/Button/Button.h>
#include <Signal/LuaSignal.h>
#include <dBrender/PostProcessing/Shake/CameraShake.h>
#include <Components/AISystem/AISystem.h>
#include <Components/Control/Sprite/Sprite.h>
#include <Components/Control/Text/Text.h>



LuaComponent::LuaComponent() {

    Signals::ReloadScript.connect(this->GetUUID(), [this](std::string path) {
        if (this->scriptPath == path) {
            ReloadScript();
        }
    });

    Signals::Editor_SetPlayMode.connect(this->GetUUID(), [this](bool state) {
        if (state) {
            calledOnReady = false;
        }
    });

    Signals::CursorOffsetChanged.connect(this->GetUUID(), [this](float x, float y)
        {
            if (!enabled)
                return;

            if (Ref::SceneLoading)
                return;

            sol::protected_function onMouseMotion = L["onMouseMotion"];
            if (onMouseMotion) {
                sol::protected_function_result result = (onMouseMotion)(x, y);
                if (!result.valid() && !onMouseMotionErr) {
                    sol::error err = result;
                    std::cerr << "[Lua Error] [Script: " << scriptName << "] onMouseMotion call failed: " << err.what()
                              << std::endl;
                    onMouseMotionErr = true;
                }
            }
        });
    name = "Lua Script";
    icon = ICON_FA_FILE_CODE_O;

}

LuaComponent::LuaComponent(const std::string &scriptPath) {
    name = "Lua Script";

    Signals::ReloadScript.connect(this->GetUUID(), [this](std::string path) {
        if (this->scriptPath == path) {
            ReloadScript();
        }
    });

    Signals::Editor_SetPlayMode.connect(this->GetUUID(), [this](bool state) {
        if (state) {
            calledOnReady = false;
        }
    });

    Signals::CursorOffsetChanged.connect(this->GetUUID(), [this](float x, float y)
        {
            if (!enabled)
                return;

            if (Ref::SceneLoading)
                return;

            sol::protected_function onMouseMotion = L["onMouseMotion"];
            if (onMouseMotion) {
                sol::protected_function_result result = (onMouseMotion)(x, y);
                if (!result.valid() && !onMouseMotionErr) {
                    sol::error err = result;
                    std::cerr << "[Lua Error] [Script: " << scriptName << "] onMouseMotion call failed: " << err.what()
                              << std::endl;
                    onMouseMotionErr = true;
                }
            }

        });
    SetScript(scriptPath);
}

void LuaComponent::SetScript(const std::string& path)
{
    onMouseMotionErr = false;
    onUpdateErr = false;
    onReadyErr = false;
    disconnectSignalsErr = false;
    connectSignalsErr = false;

    for (auto& key : global_signals)
    {
        key.second.disconnect(uuid);
    }


    scriptPath = path;
    scriptName = std::filesystem::path(scriptPath).filename().string();


    L.open_libraries(sol::lib::base, sol::lib::package, sol::lib::table, sol::lib::jit, sol::lib::math, sol::lib::string);

    L["package"]["path"] = L["package"]["path"].get<std::string>()
        + ";res/scripts/?.lua;res/scripts/?/init.lua";

    L.new_usertype<glm::vec3>(
        "vec3",
        "new", sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z
    );

    L.new_usertype<glm::vec4>(
        "vec4",
        "new", sol::constructors<glm::vec4(), glm::vec4(float, float, float, float)>(),
        "x", &glm::vec4::x,
        "y", &glm::vec4::y,
        "z", &glm::vec4::z,
        "w", &glm::vec4::w
    );

    L.new_usertype<glm::vec2>(
        "vec2",
        "new", sol::constructors<glm::vec2(), glm::vec2(float, float)>(),
        "x", &glm::vec2::x,
        "y", &glm::vec2::y
    );

    L.new_usertype<glm::quat>(
        "quat",
        "new", sol::constructors<glm::quat(), glm::quat(float, float, float, float)>(),
        "x", &glm::quat::x,
        "y", &glm::quat::y,
        "z", &glm::quat::z,
        "w", &glm::quat::w
    );
    auto quat_fromEuler = [](float pitch, float yaw, float roll) -> glm::quat
        {
            return glm::quat(glm::vec3(pitch, yaw, roll));
        };

    auto slerp = [](const glm::quat& a, const glm::quat& b, float t) -> glm::quat
        {
            t = glm::clamp(t, 0.0f, 1.0f);

    // Compute dot product to find the shortest path
            float dot = glm::dot(a, b);
            glm::quat bAdjusted = b;

            // If dot is negative, use the opposite quaternion for shortest path
            if (dot < 0.0f)
            {
                bAdjusted = -b;
                dot = -dot;
            }

            // If quaternions are very close, use linear interpolation to avoid numerical instability
            const float threshold = 0.9995f;
            if (dot > threshold)
            {
                glm::quat result = a + t * (bAdjusted - a);
                return glm::normalize(result);
            }

            // Compute slerp
            float theta = glm::acos(dot);
            float sinTheta = glm::sin(theta);
            float weightA = glm::sin((1.0f - t) * theta) / sinTheta;
            float weightB = glm::sin(t * theta) / sinTheta;

            return glm::normalize(weightA * a + weightB * bAdjusted);
        };

    L["mathf"] = L.create_table_with(
        "fromEuler", &quat_fromEuler,
        "slerp", &slerp,
        "clamp", [](float x, float min, float max)
        {
            return glm::clamp(x, min, max);
        },
        "atan2", [](float y, float x)
        {
            return glm::atan(y, x);
        },

        "lerp", [](float a, float b, float t)
        {
            return Util::lerp(a, b, t);
        },

        "multiplyQuats", [](glm::quat a, glm::quat b)
        {
            return a * b;
        }
    );

    // Post processing table

    L["PostProcessing"] = L.create_table_with(
        "TriggerCameraShake", [](float intensity, float duration)
        {
            CameraShake::GetInstance().TriggerShake(intensity, duration);
        }
    );

    L["Input"] = L.create_table_with(
        "IsActionJustPressed", [](std::string action)
        {
            return Input::GetInstance().IsActionJustPressed(action);
        },
        "IsActionPressed", [](std::string action)
        {
            return Input::GetInstance().IsActionPressed(action);
        },
        "IsActionReleased", [](std::string action)
        {
            return Input::GetInstance().IsActionReleased(action);
        },
        "GetMousePosition", []()
        {
            return Input::GetInstance().GetMousePosition();
        },
        "IsCursorLocked", []()
        {
            return Input::GetInstance().m_cursorLocked;
        },
        "SetCursorLocked", [](bool set)
        {
            Input::GetInstance().SetCursorLocked(set);
        }
    );

    L["Engine"] = L.create_table_with(
        "GetSettings", []()
        {
            return EngineSettings::GetEngineSettings();
        },
        "UseCamera", [](std::string uuid, bool useLerp)
        {
            Signals::Engine_UseCamera.emit(uuid, useLerp);
        },
        "Quit", []()
        {
            Input::GetInstance().Quit();
        }
    );

    L.new_usertype<EngineSettings>("EngineSettings",

        "glitchShaderEnabled", &EngineSettings::enableGlitchShader,
        "glitchEffectIntensity", &EngineSettings::glitchEffectIntensity,
        "deathShaderEnabled", &EngineSettings::enableGlitchEffect,
        "glitchEffectIntensity", &EngineSettings::glitchEffectIntensity,
        "glitchEffectFrequency", &EngineSettings::glitchEffectFrequency,
        "vignetteStrength", &EngineSettings::vignetteStrength,
        "fisheyeStrength", &EngineSettings::fisheyeStrength,
        "chromaticStrength", &EngineSettings::chromaticStrength,
        "shadowColor", &EngineSettings::shadowColor,
        "shadowRadius", &EngineSettings::shadowRadius,
        "shadowOffset", &EngineSettings::shadowOffset,
        "shadowIntensity", &EngineSettings::shadowIntensity,
        "displacementStrength", &EngineSettings::displacementStrength,
        "displacementSpeed", &EngineSettings::displacementSpeed,
        "scanlineHeight", &EngineSettings::scanlineHeight,
        "scanlineProbability", &EngineSettings::scanlineProbability,
        "distortionEnabled", &EngineSettings::distortionEnabled,

        "GetScanlineHeight", []()
        {
            return EngineSettings::GetEngineSettings().scanlineHeight;
        },

        "GetScanlineProbability", []()
        {
            return EngineSettings::GetEngineSettings().scanlineProbability;
        },

        "GetDisplacementStrength", []()
        {
            return EngineSettings::GetEngineSettings().displacementStrength;
        },
        "GetDisplacementSpeed", []()
        {
            return EngineSettings::GetEngineSettings().displacementSpeed;
        },
        "GetShadowColor", []()
        {
            return EngineSettings::GetEngineSettings().shadowColor;
        },
        "GetShadowRadius", []()
        {
            return EngineSettings::GetEngineSettings().shadowRadius;
        },
        "GetShadowOffset", []()
        {
            return EngineSettings::GetEngineSettings().shadowOffset;
        },
        "GetShadowIntensity", []()
        {
            return EngineSettings::GetEngineSettings().shadowIntensity;
        },
        "GetFisheyeStrength", []()
        {
            return EngineSettings::GetEngineSettings().fisheyeStrength;
        },
        "GetChromaticStrength", []()
        {
            return EngineSettings::GetEngineSettings().chromaticStrength;
        },
        "GetVignetteStrength", []()
        {
            return EngineSettings::GetEngineSettings().vignetteStrength;
        },
        "GetGlitchEffectIntensity", []()
        {
            return EngineSettings::GetEngineSettings().glitchEffectIntensity;
        },
        "GetGlitchEffectFrequency", []()
        {
            return EngineSettings::GetEngineSettings().glitchEffectFrequency;
        },
        "IsEditorEnabled", []() 
        { 
            return EngineSettings::GetEngineSettings().EditorEnabled;
        },

        "SetScanlineHeight", [](float height) { EngineSettings::GetEngineSettings().scanlineHeight = height; },
        "SetScanlineProbability", [](float probability) { EngineSettings::GetEngineSettings().scanlineProbability = probability; },
        "SetDisplacementStrength", [](float strength) { EngineSettings::GetEngineSettings().displacementStrength = strength; },
        "SetDisplacementSpeed", [](float speed) { EngineSettings::GetEngineSettings().displacementSpeed = speed; },
        "SetShadowColor", [](const glm::vec3 &color) { EngineSettings::GetEngineSettings().shadowColor = color; },
        "SetShadowRadius", [](float radius) { EngineSettings::GetEngineSettings().shadowRadius = radius; },
        "SetShadowOffset", [](const glm::vec2 &offset) { EngineSettings::GetEngineSettings().shadowOffset = offset; },
        "SetShadowIntensity", [](float intensity) { EngineSettings::GetEngineSettings().shadowIntensity = intensity; },
        "SetFisheyeStrength", [](float strength) { EngineSettings::GetEngineSettings().fisheyeStrength = strength; },
        "SetChromaticStrength", [](float strength) { EngineSettings::GetEngineSettings().chromaticStrength = strength; },
        "SetVignetteStrength", [](float strength) { EngineSettings::GetEngineSettings().vignetteStrength = strength; },
        "SetGlitchEffectIntensity", [](float intensity) { EngineSettings::GetEngineSettings().glitchEffectIntensity = intensity; },
        "SetGlitchEffectFrequency", [](float frequency) { EngineSettings::GetEngineSettings().glitchEffectFrequency = frequency; },
        "SetGlitchShaderEnabled", [](bool enabled) { EngineSettings::GetEngineSettings().enableGlitchShader = enabled; },
        "SetGlitchEffectEnabled", [](bool enabled) { EngineSettings::GetEngineSettings().enableGlitchEffect = enabled; },


        "ApplyShaderProperties", []()
        {
            Signals::PostProcessing_ApplyProperties.emit();
        }
    );

    //

    L["uuid"] = uuid;

    prepareTypes();

    bindExport();

    ReloadScript();

    Register();

}

void LuaComponent::RemoveScript()
{
    scriptName.clear();
    scriptPath.clear();
    luaComponentContainer.clear();
    m_ExposedVars.clear();
    L = sol::state{};
    L.globals().clear();
    L.collect_garbage();
}

LuaComponent::~LuaComponent() {
    //sol::optional<sol::function> disconnectSignals = L["disconnectSignals"];
    //if (disconnectSignals && disconnectSignals.value().get_type() == sol::type::function) {
    //    try {
    //        disconnectSignals.value()();
    //    } catch (const sol::error &e) {
    //        std::cerr << "Lua error calling disconnectSignals: " << e.what() << std::endl;
    //    }
    //}
    sol::protected_function disconnectSignals = L["disconnectSignals"];
    if (disconnectSignals) {
        sol::protected_function_result result = (disconnectSignals)();
        if (!result.valid() && !disconnectSignalsErr) {
            sol::error err = result;
            std::cerr << "[Lua Error] [Script: " << scriptName << "] disconnectSignals call failed: " << err.what()
                      << std::endl;
            disconnectSignalsErr = true;
        }
    }

    for (auto &key: global_signals) {
        key.second.disconnect(uuid);
    }

    luaComponentContainer.clear(); // Destroy Lua-bound objects first
    m_ExposedVars.clear();
 
    
    L.globals().clear();
    L.collect_garbage();
    //L = sol::state{};

    Signals::ReloadScript.disconnect(this->GetUUID());
    Signals::Editor_SetPlayMode.disconnect(this->GetUUID());
    Signals::CursorOffsetChanged.disconnect(this->GetUUID());
}

std::vector<ExposedVar> &LuaComponent::GetExposedVars() { return m_ExposedVars; }


void LuaComponent::ReloadScript() {
    m_ExposedVars.clear();

 /*       sol::optional<sol::function> disconnectSignals = L["disconnectSignals"];
    if (disconnectSignals && disconnectSignals.value().get_type() == sol::type::function) {
        try {
            disconnectSignals.value()();
        } catch (const sol::error &e) {
            std::cerr << "Lua error calling disconnectSignals: " << e.what() << std::endl;
        }
    }*/

    sol::protected_function disconnectSignals = L["disconnectSignals"];
    if (disconnectSignals) {
        sol::protected_function_result result = (disconnectSignals)();
        if (!result.valid() && !disconnectSignalsErr) {
            sol::error err = result;
            std::cerr << "[Lua Error] [Script: " << scriptName << "] disconnectSignals call failed: " << err.what()
                      << std::endl;
            disconnectSignalsErr = true;
        }
    }

    callOnReady();

    try {
        L.script_file(scriptPath);
    } catch (const sol::error &e) {
        std::cerr << "Error loading script: " << e.what() << std::endl;
    }

    //sol::optional<sol::function> connectSignals = L["connectSignals"];
    //if (connectSignals && connectSignals.value().get_type() == sol::type::function) {
    //    try {
    //        connectSignals.value()();
    //    } catch (const sol::error &e) {
    //        std::cerr << "Lua error calling connectSignals: " << e.what() << std::endl;
    //    }
    //}

    sol::protected_function connectSignals = L["connectSignals"];
    if (connectSignals) {
        sol::protected_function_result result = (connectSignals)();
        if (!result.valid() && !connectSignalsErr) {
            sol::error err = result;
            std::cerr << "[Lua Error] [Script: " << scriptName << "] connectSignals call failed: " << err.what()
                      << std::endl;
            connectSignalsErr = true;
        }
    }



    // seed them with default values
    for (auto &ev: m_ExposedVars) {
        L[ev.name] = ev.defaultValue;
    }

    onMouseMotionErr = false;
    onUpdateErr = false;
    onReadyErr = false;
    connectSignalsErr = false;
    disconnectSignalsErr = false;
}


void LuaComponent::callOnReady() {
    //sol::protected_function onReady = L["onReady"];
    //sol::protected_function_result result = onReady();
    //if (!result.valid() && !onReadyErr)
    //{
    //    sol::error err = result;
    //    std::cerr << "[Lua Error] [Script: " << scriptName << "] onReady call failed: " << err.what() << std::endl;
    //    onReadyErr = true;
    //}
    try
    {
        sol::protected_function onReady = L["onReady"];
        if (onReady)
        {
            sol::protected_function_result result = (onReady)();
            if (!result.valid() && !onReadyErr)
            {
                sol::error err = result;
                std::cerr << "[Lua Error] [Script: " << scriptName << "] onReady call failed: " << err.what()
                    << std::endl;
                onReadyErr = true;
            }
        }
    } catch (const sol::error &err) {
        std::cerr << "[Lua Error] [Script: " << scriptName << "] onReady call failed: " << err.what() << std::endl;
        onReadyErr = true;
    }
    catch (...)
    {
        std::cerr << "[Lua Error] [Script: " << scriptName << "] onReady call failed: Unknown error" << std::endl;
        onReadyErr = true;
    }
    
}

void LuaComponent::Update(float deltaTime) {
    if (!enabled)
        return;

    if (!calledOnReady) {
        callOnReady();
        calledOnReady = true;
    }

    if (scriptPath.empty())
        return;

    L["collectgarbage"]("stop");

    //sol::protected_function onUpdate = L["onUpdate"];
    //sol::protected_function_result result = onUpdate(deltaTime);
    //if (!result.valid() && !onUpdateErr) {
    //    sol::error err = result;
    //    std::cerr << "[Lua Error] [Script: " << scriptName << "] onUpdate call failed: " << err.what() << std::endl;
    //    onUpdateErr = true;
    //}

    sol::protected_function onUpdate = L["onUpdate"];
    if (onUpdate) {
        try
        {
            sol::protected_function_result result = (onUpdate)(deltaTime);
            if (!result.valid() && !onUpdateErr)
            {
                sol::error err = result;
                std::cerr << "[Lua Error] [Script: " << scriptName << "] onUpdate call failed: " << err.what()
                    << std::endl;
                onUpdateErr = true;
            }
        }
        catch (const sol::error& err)
        {
            std::cerr << "[Lua Error] [Script: " << scriptName << "] onUpdate call failed: " << err.what() << std::endl;
            onUpdateErr = true;
        }
        catch (...)
        {
            std::cerr << "[Lua Error] [Script: " << scriptName << "] onUpdate call failed: Unknown error" << std::endl;
            onUpdateErr = true;
        }
        
    }
    


    L["collectgarbage"]("collect");
}

void LuaComponent::Register() {


    L.new_usertype<LuaComponent>("LuaComponent", "UpdateCameraVectors", &LuaComponent::Lua_UpdateCameraVectors, "Quit",
                                 &LuaComponent::Lua_Quit);



   
    L["dBengine"] = this;

    //luaComponentContainer.push_back(std::move(std::make_unique<Lua_GameObject>(this)));
    //luaComponentContainer.push_back(std::move(std::make_unique<Lua_Input>(this)));
    //luaComponentContainer.push_back(std::move(std::make_unique<Lua_Camera>(this)));
    //luaComponentContainer.push_back(std::move(std::make_unique<Lua_TextRenderer>(this)));
    //luaComponentContainer.push_back(std::move(std::make_unique<Lua_Animator>(this)));
    //luaComponentContainer.push_back(std::move(std::make_unique<Lua_ParticleSystem>(this)));
}

void LuaComponent::Lua_UpdateCameraVectors() {
    // Signals::UpdateCameraVectors.emit();
}

void LuaComponent::Lua_Quit() { Input::GetInstance().Quit(); }

void LuaComponent::bindExport() {

    m_ExposedVars.clear();

    L.set_function("export", sol::overload(

        [this](const std::string &name, sol::object defaultValue) {
                                         m_ExposedVars.push_back({name, defaultValue});
        },

        [this](const std::string &name, float x, float y, float z) {
            // construct a glm::vec3 in Lua
            glm::vec3 v{x, y, z};
            sol::object obj = sol::make_object(L, v);
            m_ExposedVars.push_back({name, obj});
        }
        
    ));

}



sol::object GetComponentByName(GameObject* self, const std::string& componentName, sol::this_state s)
{
    sol::state_view lua(s);
    if (!self)
        sol::make_object(lua, sol::nil);
    if (componentName == "Button")
    {
        if (auto* b = self->GetComponent<UI::Button>())
        {
            return sol::make_object(lua, b);
        }
    }
    else if (componentName == "ParticleSystem")
    {
        if (auto* ps = self->GetComponent<ParticleSystem>())
        {
            return sol::make_object(lua, ps);
        }
    } else if (componentName == "AudioSource") {
        if (auto *as = self->GetComponent<AudioSource>()) {
            return sol::make_object(lua, as);
        }
    } 
    else if (componentName == "Animator")
    {
        if (auto* an = self->GetComponent<Animator>())
        {
            return sol::make_object(lua,an);
        }
    }
    else if (componentName == "Tag")
    {
        if (auto* tag = self->GetComponent<Tag>())
        {
            return sol::make_object(lua, tag);
        }
    }
    else if (componentName == "CollisionShape")
    {
        if (auto* cs = self->GetComponent<CollisionShape>())
        {
            return sol::make_object(lua, cs);
        }
    }
    else if (componentName == "MeshInstance")
    {
        if (auto* ms = self->GetComponent<MeshInstance>())
        {
            return sol::make_object(lua, ms);
        }
    }
    else if (componentName == "AIAgent") {
        if (auto *aia = self->GetComponent<AIAgent>()) {
            return sol::make_object(lua, aia);
        }
    } 
    else if (componentName == "AISystem") {
        if (auto *ais = self->GetComponent<AISystem>()) {
            return sol::make_object(lua, ais);
        }
    } 
    else if (componentName == "NavigationTarget") {
        if (auto *nt = self->GetComponent<NavigationTarget>()) {
            return sol::make_object(lua, nt);
        }
    }
    else if (componentName == "PhysicsBody") 
    {
        if (auto *pb = self->GetComponent<PhysicsBody>()) 
        {
            return sol::make_object(lua, pb);
        }
    } 
    else if (componentName == "Sprite")
    {
        if (auto* spr = self->GetComponent<UI::Sprite>())
        {
            return sol::make_object(lua, spr);
        }
    }
    else if (componentName == "Text")
    {
        if (auto* txt = self->GetComponent<UI::Text>())
        {
            return sol::make_object(lua, txt);
        }
    }
    else if (componentName == "Camera")
    {
        if (auto* cam = self->GetComponent<Camera>())
        {
            return sol::make_object(lua, cam);
        }
    }
    else if (componentName == "PointLight")
    {
        if (auto* light = self->GetComponent<PointLight>())
        {
            return sol::make_object(lua, light);
        }
    }
    else if (componentName == "SpotLight")
    {
        if (auto *light = self->GetComponent<SpotLight>())
        {
            return sol::make_object(lua, light);
        }
    }

    return sol::make_object(lua, sol::nil);
}


void LuaComponent::prepareTypes()
{
    L.new_usertype<Signal<>>("Signal",
        "connect", &Signal<>::connect,
        "emit", &Signal<>::emit,
        "disconnect", &Signal<>::disconnect
    );

    sol::table signals_table = L.create_table();
    signals_table["Timer_OnTimeout"] = &Signals::Timer_OnTimeout;
    L["signals"] = signals_table;

    /*sol::optional<sol::function> connectSignals = L["connectSignals"];
    if (connectSignals && connectSignals.value().get_type() == sol::type::function) {
        try {
            connectSignals.value()();
        } catch (const sol::error &e) {
            std::cerr << "Lua error calling connectSignals: " << e.what() << std::endl;
        }
    }*/

    //types
    L.new_usertype<GameObject>("GameObject",
        "name", &GameObject::name,
        "uuid", &GameObject::uuid,
        "transform", &GameObject::transform,
        "GetChildren", [](GameObject* self)
        {
            std::vector<GameObject*> children;
            for (const auto& child : self->children)
            {
                children.push_back(child.get());
            }

            return sol::as_table(children);
        },
        "GetChildrenCount", [](GameObject* self)
        {
            return self->children.size();
        },
        "GetChild", &GameObject::GetChild,
        "GetChildUUID", &GameObject::GetChildUUID,
        "GetComponent", &GetComponentByName,
        "Enable", &GameObject::Enable,
        "Disable", &GameObject::Disable,
        "enabled", &GameObject::m_enabled,
        "parent", &GameObject::parent, 
        "GetUUID", &GameObject::GetUUID
    );

    L.new_usertype<Transform>("Transform",
        "globalPosition", &Transform::globalPosition,
        "localPosition", &Transform::position,
        "GetEulerRotation", &Transform::GetEulerRotation,
        "SetLocalPosition", &Transform::SetLocalPosition,
        "GetLocalPosition", &Transform::GetLocalPosition,
        "SetEulerRotation", &Transform::SetEulerRotation,
        "SetGlobalPosition", &Transform::SetGlobalPosition,
        "GetGlobalPosition", &Transform::GetGlobalPosition,
        "SetScale", &Transform::SetScale,
        "GetScale", &Transform::GetScale,
        "scale", &Transform::scale,
        "SetQuatRotation", &Transform::SetQuatRotation,
        "GetQuatRotation", &Transform::GetQuatRotation
    );

    L.new_usertype<Scene>("Scene",
        "root", [](Scene* self)
        {
            return self->sceneRootObject.get();
        },
        "GetGameObject", &Scene::GetGameObject,
        "GetGameObjectUUID", &Scene::GetGameObjectUUID, 
        "DeleteGameObject", &Scene::DeleteGameObject,
        "name", &Scene::name,
        "path", &Scene::Path

    );

    L.new_usertype<AudioSource>("AudioSource", 
        "SetPitch", &AudioSource::SetPitch, 
        "SetPitchAll", &AudioSource::SetPitchAll,
        "SetVolume", &AudioSource::SetVolume,
        "SetPitchAll", &AudioSource::SetVolumeAll,
        "AddToEvents", &AudioSource::AddToEvents,
        "PlayWithVariation", &AudioSource::PlayWithVariation,
        "Play", &AudioSource::Play,
        "SetParameter", &AudioSource::SetParameter
    );

    L.new_usertype<UI::Control>("Control",
        "position", &UI::Control::Position,
        "size", &UI::Control::Size,
        "rotation", &UI::Control::Rotation, 
        "emission", &UI::Control::Emission
    );

    L.new_usertype<UI::Button>("Button",
        sol::base_classes, sol::bases<UI::Control>(),
        "pressed", &UI::Button::Pressed
    );

    L.new_usertype<UI::Sprite>("Sprite",
        sol::base_classes, sol::bases<UI::Control>(),
        "clipping", &UI::Sprite::Clipping, // x = left, y = right, z = bottom, w = top
        "modulate", &UI::Sprite::ModulateColor
    );

    L.new_usertype<UI::Text>("Text",
        sol::base_classes, sol::bases<UI::Control>(),
        "GetText", &UI::Text::GetText,
        "SetText", &UI::Text::SetText
    );

    L.new_usertype<PlayerController>("PlayerController", "moveSpeed", &PlayerController::moveSpeed, "acceleration",
                                     &PlayerController::acceleration);

    L.new_usertype<Hitbox>("Hitbox", "SetActive", &Hitbox::SetActive, "IsActive", &Hitbox::IsActive, "SetDamage",
                           &Hitbox::SetDamage, "GetDamage", &Hitbox::GetDamage);
  

    L.new_usertype<ParticleSystem>("ParticleSystem",
        "Emit", &ParticleSystem::Emit,
        "Stop", &ParticleSystem::Stop,
        "IsEmitting", &ParticleSystem::IsEmitting,
        "oneShot", &ParticleSystem::OneShot

    );

    L.new_usertype<Animator>("Animator",
        "PlayAnimation", &Animator::PlayAnimation,
        "PlayTransition", &Animator::PlayTransition,
        "Play", &Animator::Play,
        "Pause", &Animator::Pause,
        "Reset", &Animator::Reset, 
        "SetTimeScale", &Animator::SetTimeScale,
        "GetAnimationProgress", &Animator::GetAnimationProgress
    );

    L.new_usertype<Tag>("Tag",
        "name", &Tag::Name
    );

    L.new_usertype<CollisionShape>("CollisionShape",
        "IsCollisionArea", &CollisionShape::GetIsCollisionArea,
        "gameObjectsInArea", &CollisionShape::gameObjectsInArea,
        "SetIsCollisionArea", &CollisionShape::SetIsCollisionArea, 
        "Disable", &CollisionShape::Disable
        
    );

    L.new_usertype<MeshInstance>("MeshInstance",
        "IsVolumetric", &MeshInstance::isUsingVolumetric,
        "density", &MeshInstance::density,
        "samples", &MeshInstance::samples,
        "fogColor", &MeshInstance::fogColor,
        "scattering", &MeshInstance::scattering
    );

    L.new_usertype<PhysicsBody>("PhysicsBody",
        "ApplyForce", &PhysicsBody::ApplyForce,
        "SetStatic", &PhysicsBody::SetStatic,
        "SetMass", &PhysicsBody::SetMass, 
        "Disable", &PhysicsBody::Disable,
        
        "mass", &PhysicsBody::mass,
        "useGravity", &PhysicsBody::useGravity,
        "velocity", &PhysicsBody::velocity,
        "isStatic", &PhysicsBody::isStatic,
        "isGrounded", &PhysicsBody::isGrounded,
        "linearDamping", &PhysicsBody::linearDamping

    );

    L.new_usertype<AISystem>("AISystem", 
        "SetNavigationMesh", &AISystem::SetNavigationMesh, 
        "GetNavigationMesh", &AISystem::GetNavigationMesh, 
        "RegisterAgent", &AISystem::RegisterAgent, 
        "RemoveAgent", &AISystem::RemoveAgent, 
        "SetTarget", &AISystem::SetTarget, 
        "GetTarget", &AISystem::GetTarget, 
        "GetAgents", &AISystem::GetAgents,  
        "SetCurrentAttacker", &AISystem::SetCurrentAttacker, 
        "GetCurrentAttacker", &AISystem::GetCurrentAttacker, 
        "HasLineOfSightWithPlayer", &AISystem::HasLineOfSightWithPlayer, 
        "UpdatePathsBulk", &AISystem::UpdatePathsBulk, 
        "CalculateDistance", &AISystem::CalculateDistance, 
        "InitializeCircleAngles", &AISystem::InitializeCircleAngles);

    L.new_usertype<AIAgent>("AIAgent",
        "GetPosition", &AIAgent::GetPosition,
        "GetVelocity", &AIAgent::GetVelocity,
        "GetState", &AIAgent::GetState, 
        "SetState", &AIAgent::SetState, 
        "GetTargetPos", &AIAgent::GetTargetPos,
        "SetTargetPos", &AIAgent::SetTargetPos,
        "SetPath", &AIAgent::SetPath, 
        "GetPath", &AIAgent::GetPath, 
        "ClearPath", &AIAgent::ClearPath,
        "SetSpeed", &AIAgent::SetSpeed, 
        "GetSpeed", &AIAgent::GetSpeed, 
        "SetMaxSpeed", &AIAgent::SetMaxSpeed, 
        "GetMaxSpeed", &AIAgent::GetMaxSpeed, 
        "SetStoppingDistance", &AIAgent::SetStoppingDistance, 
        "GetStoppingDistance", &AIAgent::GetStoppingDistance,
        "SetLineOfSightDistance", &AIAgent::SetLineOfSightDistance, 
        "GetLineOfSightDistance", &AIAgent::GetLineOfSightDistance, 
        "Pause", &AIAgent::Pause, 
        "Resume", &AIAgent::Resume,
        "IsPaused", &AIAgent::IsPaused, 
        "GetGameObject", &AIAgent::GetGameObject, 
        "GetUUID", &AIAgent::GetUUID);

    L.new_enum<AIAgentState>("AIAgentState", {{"IDLE", AIAgentState::IDLE},
                                              {"SEEK", AIAgentState::SEEK},
                                              {"FLEE", AIAgentState::FLEE},
                                              {"WANDER", AIAgentState::WANDER},
                                              {"PATH_FOLLOWING", AIAgentState::PATH_FOLLOWING},
                                              {"ATTACK", AIAgentState::ATTACK},
                                              {"CIRCLING", AIAgentState::CIRCLING}});

    L.new_usertype<NavigationTarget>("NavigationTarget", "GetPosition", &NavigationTarget::GetPosition);
    L.new_usertype<Camera>("Camera",
        "isUsed", &Camera::isUsed,
        "front", &Camera::Front,
        "right", &Camera::Right,
        "pitch", &Camera::Pitch,
        "yaw", &Camera::Yaw,
        "UpdateCameraVectors", &Camera::UpdateCameraVectors,
        "UpdateCamera", [this](float yaw, float pitch)
        {
            Camera* camera_component = gameObject->GetComponent<Camera>();
            
            if (camera_component && camera_component->isUsed)
            {
                camera_component->Yaw = yaw;
                camera_component->Pitch = pitch;
                camera_component->UpdateCameraVectors();
            }
        }
    );

    L.new_usertype<PointLight>("PointLight",
        "intensity", &PointLight::intensity
    );

    L.new_usertype<SpotLight>("SpotLight",
        "intensity", &SpotLight::intensity
    );

    L.new_usertype<Ref>("Ref", "playerHealth", &Ref::playerHealth, "bossHealth", &Ref::bossHealth);

    L.set_function("GetRef", []() { return &Ref::GetInstance(); });

    L.set_function("CreateGameObject", [](const std::string& name)
        {
            auto gameObject = dBrender::GetInstance().m_activeScene->CreateGameObject(name);
            return gameObject;
        });

    L.set_function("AddChild", [this](const std::string &pathToScene) {
        Signals::FileToGameObject.emit(pathToScene, gameObject);
        return gameObject->children.back().get();
    });

    L.set_function("GetChildren", [this]()
        {
            std::vector<GameObject*> children;

            for (const auto& child : gameObject->children)
            {
                children.push_back(child.get());
            }

            return sol::as_table(children);
        });

    L.set_function("GetChildrenCount", [this]()
        {
            return gameObject->children.size();
        });

    L.set_function("GetScene", [this]()
        {
            return Ref::CurrentScene;
        });

    L.set_function("GetChild", [this](const std::string& childName)
        {
            return gameObject->GetChild(childName);
        });

    L.set_function("GetComponent", &GetComponentByName);

    L.set_function("self", [this]() { return gameObject; });

    L.set_function("LoadScene", [this](const std::string& path)
        {
            Signals::Engine_PrepareSceneLoad.emit(path);
            //RemoveScript();
        });

    //// Signals
    L.new_usertype<LuaSignal>("LuaSignal", 
        "connect", &LuaSignal::connect, 
        "emit", &LuaSignal::emit, 
        "disconnect", &LuaSignal::disconnect
    );

    L.set_function("connect_signal", [this](const std::string &signal_name, const std::string &uuid,
                                           sol::function slot) {
        global_signals[signal_name].connect(uuid, slot);
        m_SignalNames.push_back(signal_name);
    });

    L.set_function("emit_signal",
                   [](const std::string &signal_name, sol::variadic_args args) {
                       global_signals[signal_name].emit(args);
                   });

    L.set_function("disconnect_signal", [](const std::string &signal_name, const std::string &uuid) {
        global_signals[signal_name].disconnect(uuid);
    });



}