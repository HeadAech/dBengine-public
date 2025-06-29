#include "Lua_ParticleSystem.h"
#include <dBengine/EngineDebug/EngineDebug.h>

ParticleSystem *Lua_ParticleSystem::getParticleSystem() {
    return lua_component->gameObject->GetComponent<ParticleSystem>();
}


Lua_ParticleSystem::Lua_ParticleSystem(LuaComponent *lua_component) : Lua(lua_component)
{

    this->uuid = UUID::generateUUID();
    Register();

}

void Lua_ParticleSystem::Register() 
{

    lua_component->L.new_usertype<Lua_ParticleSystem>("ParticleSystem",

        "Emit", &Lua_ParticleSystem::Emit,
        "Stop", &Lua_ParticleSystem::Stop

    );

    lua_component->L["ParticleSystem"] = this;
}

void Lua_ParticleSystem::Emit() 
{
    if (auto ps = getParticleSystem())
    {
        ps->Emit();
    }
    else
    {
        EngineDebug::GetInstance().PrintError("Couldnt emit, no particle system");
    }

}

void Lua_ParticleSystem::Stop()
{
    if (auto ps = getParticleSystem())
    {
        ps->Stop();
    }
    else
    {
        EngineDebug::GetInstance().PrintError("Couldn't stop emitting, no particle system");
    }
}


