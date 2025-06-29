#ifndef LUA_ANIMATOR_H
#define LUA_ANIMATOR_H

#include <sol.hpp>
#include "Components/LuaComponent/Lua.h"
#include "Components/Animator/Animator.h"

class Lua_Animator : public Lua {

    Animator *getAnimator();

	public:

		Lua_Animator() = default;
        Lua_Animator(LuaComponent *lua_component);
        ~Lua_Animator() = default;
        void Register() override;

        void PlayAnimation(const std::string &name);

        void Play();
        void Pause();

        void PlayTransition(const std::string &name);

        void SetBlendFactor(float blendFactor);
        float GetBlendFactor();

        void SetTimeScale(float timeScale);
        float GetTimeScale();

        const std::string GetCurrentAnimationName();
};


#endif // !LUA_ANIMATOR_H
