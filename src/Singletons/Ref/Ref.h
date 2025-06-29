#ifndef REF_H
#define REF_H

#include <glm/glm.hpp>
#include "Scene/Scene.h"
#include "fmod.hpp"
#include "fmod_studio.hpp"

/// <summary>
/// Utility class for reference values.
/// </summary>
class Ref
{
	public:
		Ref() = default;
		//singleton
        Ref(const Ref &) = delete;
        Ref &operator=(const Ref &) = delete;
        Ref(Ref &&) = default;

        /// <summary>
        /// Returns the singleton instance of the Ref class.
        /// </summary>
        /// <returns>Singleton (Ref)</returns>
        static Ref &GetInstance() 
        {
            static Ref instance;
            return instance;
        }

        /// <summary>
        /// Mouse position on the viewport.
        /// </summary>
        static glm::vec2 MousePosition;

        /// <summary>
        /// Current viewport resolution.
        /// </summary>
        static glm::ivec2 ScreenResolution;

        /// <summary>
        /// Current size of the window.
        /// </summary>
        static glm::ivec2 WindowSize;

        static Scene* CurrentScene;

        static float AspectRatio;

        static bool SceneLoading;
        static FMOD_RESULT ERRCHECK(FMOD_RESULT result);
        
        float playerHealth = 1.0f;

        float bossHealth = 1.0f;

        FMOD::Studio::System *fmodSystem = NULL;
        FMOD::System *fmodCoreSystem = NULL;
        FMOD::DSP *fmodDSP = NULL;
        
        void InitializeFMOD();
        void LoadFmodBank(std::string_view bankPath);
};

#endif // !REF_H
