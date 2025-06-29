#include "Ref.h"
#include <glm/glm.hpp>
#include <Scene/Scene.h>
#include <fmod_errors.h>

glm::vec2 Ref::MousePosition = {0.0f, 0.0f};
glm::ivec2 Ref::ScreenResolution = { 1920, 1080 };
glm::ivec2 Ref::WindowSize = {1920, 1080};

Scene* Ref::CurrentScene = nullptr;

float Ref::AspectRatio = 16.0f / 9.0f; // Default aspect ratio

bool Ref::SceneLoading = false;
inline FMOD_RESULT Ref::ERRCHECK(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        EngineDebug::GetInstance().PrintError("FMOD error! (" + std::to_string(result) +"): " + FMOD_ErrorString(result));
    }
    return result;
}

void Ref::InitializeFMOD(){
    ERRCHECK(FMOD::Studio::System::create(&fmodSystem));
    ERRCHECK(fmodSystem->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0));
    fmodSystem->getCoreSystem(&fmodCoreSystem);
    fmodCoreSystem->createDSPByType(FMOD_DSP_TYPE_FFT, &fmodDSP);

    fmodDSP->setParameterInt(FMOD_DSP_FFT_WINDOW, FMOD_DSP_FFT_WINDOW_HANNING);
    fmodDSP->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, 1024); //as of now sampling is the same always.
}

void Ref::LoadFmodBank(std::string_view bankPath) {
    //can i check whether it is loaded or not?
    FMOD::Studio::Bank *bank;
    Ref::ERRCHECK(fmodSystem->loadBankFile(bankPath.data(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank));
}
