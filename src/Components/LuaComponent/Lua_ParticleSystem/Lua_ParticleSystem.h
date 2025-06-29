#ifndef LUA_PARTICLE_SYSTEM_H
#define LUA_PARTICLE_SYSTEM_H

#include <sol.hpp>
#include "Components/LuaComponent/Lua.h"
#include "Components/LuaComponent/LuaComponent.h"
#include <Components/Particles/ParticleSystem.h>

class Lua_ParticleSystem : public Lua
{
	public:
		
		Lua_ParticleSystem() = default;
        Lua_ParticleSystem(LuaComponent *lua_component);
        ~Lua_ParticleSystem() = default;

		void Register() override;

		void Emit();
        void Stop();

	private:
		
		ParticleSystem *getParticleSystem();
};	


#endif // !LUA_PARTICLE_SYSTEM_H
