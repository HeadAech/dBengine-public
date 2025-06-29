#include "Lua_Animator.h"
#include "Components/LuaComponent/LuaComponent.h"
#include <Components/Animator/Animator.h>

Animator *Lua_Animator::getAnimator() { return lua_component->gameObject->GetComponent<Animator>(); }

Lua_Animator::Lua_Animator(LuaComponent *lua_component) : Lua(lua_component) {

	this->uuid = UUID::generateUUID();
    Register();

}

void Lua_Animator::Register() {
	lua_component->L.new_usertype<Lua_Animator>("Animator",

		"PlayAnimation", &Lua_Animator::PlayAnimation, 
        "Play", &Lua_Animator::Play, 
        "Pause", &Lua_Animator::Pause, 
        "PlayTransition", &Lua_Animator::PlayTransition,
        "SetBlendFactor", &Lua_Animator::SetBlendFactor,
        "GetBlendFactor", &Lua_Animator::GetBlendFactor,
        "SetTimeScale", &Lua_Animator::SetTimeScale,
        "GetTimeScale", &Lua_Animator::GetTimeScale,
        "GetCurrentAnimationName", &Lua_Animator::GetCurrentAnimationName

    );
    lua_component->L["Animator"] = this;
}


void Lua_Animator::PlayAnimation(const std::string &name) {

	if (auto animator = getAnimator()) {
        animator->PlayAnimation(name);
    } else {
        EngineDebug::GetInstance().PrintError("Lua Animator:PlayAnimation() Error: GameObject does not have an Animator component.");
    }

}

void Lua_Animator::Play() {

    if (auto animator = getAnimator()) {
        animator->Play();
    } else {
        EngineDebug::GetInstance().PrintError("Lua Animator:Play() Error: GameObject does not have an Animator component.");
    }

}


void Lua_Animator::Pause() {

    if (auto animator = getAnimator()) {
        animator->Pause();
    } else {
        EngineDebug::GetInstance().PrintError(
                "Lua Animator:Pause() Error: GameObject does not have an Animator component.");
    }
}

void Lua_Animator::PlayTransition(const std::string &name) {
    if (auto animator = getAnimator()) {
        animator->PlayTransition(name);
    } else {
        EngineDebug::GetInstance().PrintError(
                "Lua Animator:PlayTransition() Error: GameObject does not have an Animator component.");
    }
}

void Lua_Animator::SetBlendFactor(float blendFactor) {

    if (auto animator = getAnimator()) {
        animator->SetBlendFactor(blendFactor);
    } else {
        EngineDebug::GetInstance().PrintError(
                "Lua Animator:SetBlendFactor() Error: GameObject does not have an Animator component.");
    }

}

float Lua_Animator::GetBlendFactor() {

    if (auto animator = getAnimator()) {
        return animator->GetBlendFactor();
    } else {
        EngineDebug::GetInstance().PrintError(
                "Lua Animator:GetBlendFactor() Error: GameObject does not have an Animator component.");
        return 0.0f;
    }
}

void Lua_Animator::SetTimeScale(float timeScale) {

    if (auto animator = getAnimator()) {
        animator->SetTimeScale(timeScale);
    } else {
        EngineDebug::GetInstance().PrintError(
                "Lua Animator:SetTimeScale() Error: GameObject does not have an Animator component.");
    }

}

float Lua_Animator::GetTimeScale() { 

    if (auto animator = getAnimator()) {
        return animator->GetTimeScale();
    } else {
        EngineDebug::GetInstance().PrintError(
                "Lua Animator:GetTimeScale() Error: GameObject does not have an Animator component.");
        return 0.0f;
    }

}

const std::string Lua_Animator::GetCurrentAnimationName() { 

    if (auto animator = getAnimator()) {
        if (auto animation = animator->GetCurrentAnimation()) {
            return animation->GetName();
        } else {
            EngineDebug::GetInstance().PrintError(
                    "Lua Animator:GetCurrentAnimationName() Error: Current animation is null.");
            return "";
        }
    } else {
        EngineDebug::GetInstance().PrintError(
                "Lua Animator:GetCurrentAnimationName() Error: GameObject does not have an Animator component.");
        return "";
    }

}



