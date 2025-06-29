//
// Created by Hubert Klonowski on 15/03/2025.
//
#pragma once
#ifndef SIGNALS_H
#define SIGNALS_H
#include "Signal.h"
#include <string>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <memory>


class Animation;
class GameObject;

/// <summary>
/// Class-container for signals used in the engine.
/// </summary>
class Signals {

public:

    /// <summary>
    /// Calls when offsets of the cursor change.
    /// </summary>
    static Signal<double, double> CursorOffsetChanged;

    /// <summary>
    /// Calls when the mouse motion is detected.
    /// </summary>
    static Signal<double, double> MouseMotion;

    /// <summary>
    /// Calls for update of the camera vectors.
    /// </summary>
    static Signal<> UpdateCameraVectors;

    /// <summary>
    /// Call to reload a script
    /// <param name="scriptPath">Path to the script to reload</param>
    /// </summary>
    static Signal<std::string> ReloadScript;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Calls when VSync settings change.
    /// </summary>
    static Signal<> EngineSettings_ChangedVSync;

    /// <summary>
    /// Call for creating a new game object in the scene.
    /// </summary>
    static Signal<std::string, std::string, glm::vec3, GameObject *> InstantiateGameObject;

    /// <summary>
    /// Call for creating a new, empty GameObject in the scene.
    /// </summary>
    static Signal<GameObject*> InstantiateEmptyGameObject;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for the debug console to scroll it's content to the bottom.
    /// </summary>
    static Signal<> Console_ScrollToBottom;

    /// <summary>
    /// Call for the camera to update its aspect ratio.
    /// <param name="aspectRatio">Aspect ratio to set</param>
    /// </summary>
    static Signal<float> Camera_UpdateAspectRatio;

    /// <summary>
    /// Call for editor to exit fullscreen mode.
    /// </summary>
    static Signal<> Editor_ExitFullscreen;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for editor to set the play mode.
    /// <param name="playMode">True for play mode, false for edit mode</param>
    /// </summary>
    static Signal<bool> Editor_SetPlayMode;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call to reload all shader programs.
    /// </summary>
    static Signal<> ReloadShaders;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for specific shader if it was reloaded.
    /// <param name="shaderID">ID of the shader that was reloaded</param>
    /// </summary>
    static Signal<unsigned int> ShaderReloaded;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for shaders to initialize.
    /// </summary>
    static Signal<> SetupShaders;


    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call to save the scene to file
    /// </summary>
    static Signal<std::string> SceneToFile;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call to load scene from file
    /// </summary>
    static Signal<std::string> FileToScene;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call to load scene to gameObject in current scene
    /// </summary>
    static Signal<std::string, GameObject *> FileToGameObject;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call to create/load a new empty scene
    /// </summary>
    static Signal<std::string> NewScene;


    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for editor to set its theme.
    /// <param name="themeID">ID of the theme to set</param>
    /// </summary>
    static Signal<int> Editor_SetTheme;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for the engine to set the MSAA samples.
    /// <param name="samples">Number of MSAA samples to set</param>
    /// </summary>
    static Signal<int> Engine_SetMSAASamples;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for code editor inside the engine to set the script path to edit.
    /// <param name="scriptPath">Path to the script to edit</param>
    /// </summary>
    static Signal<std::string> Editor_SetScriptPathToEdit;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for engine to validate the framebuffer dimensions.
    /// </summary>
    static Signal<> Engine_CheckFramebufferDimensions;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for engine to set window fullscreen mode.
    /// <param name="fullscreen">True for fullscreen mode, false for windowed mode</param>
    /// </summary>
    static Signal<bool> Engine_SetWindowFullscreen;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for animation library to delete an animation.
    /// <param name="animation_ptr">Shared pointer to the animation to delete</param>
    /// </summary>
    static Signal<std::shared_ptr<Animation>> AnimationLibrary_AnimationDeleted;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call when Light is added
    /// </summary>
    static Signal<std::string> Scene_AddedDirectionalLight;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call when Light is added
    /// </summary>
    static Signal<std::string> Scene_AddedSpotLight;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call when Light is added
    /// </summary>
    static Signal<std::string> Scene_AddedPointLight;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call when WE is added
    /// </summary>
    static Signal<std::string> Scene_AddedWorldEnvironment;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for changing the camera to Engine Editor one
    /// </summary>
    static Signal<std::string, bool> Engine_UseCamera;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for return to scene camera
    /// </summary>
    static Signal<bool> Engine_ReturnToSceneCamera;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for reinitializing m_flatObjects in render
    /// </summary>
    static Signal<> Render_ResetFlatObjects;

    /// <summary>
    /// <para>INTERNAL USE</para>
    /// Call for removing GameObject from m_flatObjects in render, when GO is deleted.
    /// </summary>
    static Signal<GameObject*> Render_RemoveGameObject;

    /// <summary>
    /// Timer on beat timeout.
    /// put it in timer constructor, and connect it where you want to be used.
    /// </summary>
    static Signal<> Timer_OnTimeout;
    static Signal<float> Music_AdjustPitch;
    static Signal<float> Music_AdjustVolume;

    static Signal<> Engine_LoadNextScene;

    static Signal<> Collision_OnAreaEnter;

    /// <summary>
    /// Hitbox hit signal - emitted when a hitbox hits a target
    /// Parameters: attacker GameObject*, target GameObject*, damage amount, hitbox name
    /// </summary>
    static Signal<GameObject *, GameObject *, float, std::string> Hitbox_Hit;

    static Signal<std::string> Engine_PrepareSceneLoad;

    static Signal<float, float> PostProcessing_TriggerCameraShake;

    static Signal<> PostProcessing_ApplyProperties;

    static Signal<> ForceRenderLights;
};



#endif //SIGNALS_H
