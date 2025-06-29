//
// Created by Hubert Klonowski on 15/03/2025.
//

#include "Signals.h"

Signal<double, double> Signals::CursorOffsetChanged;
Signal<double, double> Signals::MouseMotion;
Signal<> Signals::UpdateCameraVectors;

Signal<std::string> Signals::ReloadScript;

Signal<> Signals::EngineSettings_ChangedVSync;

Signal<std::string, std::string, glm::vec3, GameObject *> Signals::InstantiateGameObject;
Signal<GameObject*> Signals::InstantiateEmptyGameObject;

Signal<> Signals::Console_ScrollToBottom;

Signal<float> Signals::Camera_UpdateAspectRatio;

Signal<> Signals::Editor_ExitFullscreen;
Signal<bool> Signals::Editor_SetPlayMode;

Signal<> Signals::ReloadShaders;
Signal<unsigned int> Signals::ShaderReloaded;
Signal<> Signals::SetupShaders;

Signal<std::string> Signals::SceneToFile;
Signal<std::string> Signals::FileToScene;
Signal<std::string, GameObject *> Signals::FileToGameObject;
Signal<std::string> Signals::NewScene;


Signal<int> Signals::Editor_SetTheme;

Signal<int> Signals::Engine_SetMSAASamples;
Signal<std::string> Signals::Editor_SetScriptPathToEdit;

Signal<> Signals::Engine_CheckFramebufferDimensions;

Signal<bool> Signals::Engine_SetWindowFullscreen;
Signal<std::shared_ptr<Animation>> Signals::AnimationLibrary_AnimationDeleted;

Signal<std::string> Signals::Scene_AddedDirectionalLight;
Signal<std::string> Signals::Scene_AddedSpotLight;
Signal<std::string> Signals::Scene_AddedPointLight;
Signal<std::string> Signals::Scene_AddedWorldEnvironment;

Signal<std::string, bool> Signals::Engine_UseCamera;
Signal<bool> Signals::Engine_ReturnToSceneCamera;
Signal<> Signals::Render_ResetFlatObjects;
Signal<GameObject*> Signals::Render_RemoveGameObject;

Signal<> Signals::Timer_OnTimeout;
Signal<float> Signals::Music_AdjustPitch;
Signal<float> Signals::Music_AdjustVolume;
Signal<> Signals::Collision_OnAreaEnter;


Signal<> Signals::Engine_LoadNextScene;

Signal<GameObject *, GameObject *, float, std::string> Signals::Hitbox_Hit;

Signal<std::string> Signals::Engine_PrepareSceneLoad;

Signal<float, float> Signals::PostProcessing_TriggerCameraShake;

Signal<> Signals::PostProcessing_ApplyProperties;

Signal<> Signals::ForceRenderLights;