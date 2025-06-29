//
// Created by Hubert Klonowski on 25/03/2025.
//

#ifndef DBENGINE_H
#define DBENGINE_H
#include "Components/Camera/Camera.h"
#include "dBrender/dBrender.h"
#include "EngineDebug/EngineDebug.h"
#include "EngineGUI/EngineGUI.h"
#include "EngineSettings/EngineSettings.h"
#include "GLFW/glfw3.h"
#include "InputManager/Input.h"
#include "fmod_studio.hpp"
#include "dBphysics/dBphysics.h"
#include <Components/Animator/AnimationLibrary/AnimationLibrary.h>
#include <ResourceManager/ResourceManager.h>
#include <Serializers/SceneSerializer.h>
#include "Singletons/Ref/Ref.h"
#include "Scheduler/Scheduler.h"
#include <future>

#ifdef __APPLE__
#define HOLD_ALT ImGui::GetIO().KeyAlt // Cmd key on macOS
#else
#define HOLD_ALT ImGui::GetIO().KeyAlt // Ctrl key on Windows/Linux
#endif

class dBengine {

    Scheduler m_Scheduler;
    std::future<std::optional<std::shared_ptr<Scene>>> pendingSceneFuture;
    std::vector<std::future<std::optional<std::shared_ptr<Scene>>>> pendingSceneFutures;
    std::mutex sceneMutex;

    Input& inputManager = Input::GetInstance();

    dBrender& renderer = dBrender::GetInstance();

    EngineDebug& debug = EngineDebug::GetInstance();

    dBphysics& physics = dBphysics::GetInstance();

    EngineGUI& engineGUI = EngineGUI::GetInstance();

    Ref& ref = Ref::GetInstance();

    Serialize::SceneSerializer &sceneSerializer = Serialize::SceneSerializer::GetInstance();


    static void glfw_error_callback(int error, const char* description);

    GLFWwindow* window = nullptr;
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void window_size_callback(GLFWwindow *window, int width, int height);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float defaultAspectRatio = (16.0f / 9.0f);
    std::string uuid;

    void registerInputActions();

    // other
    int setup();
    void input();
    void update();
    void render();

    void endFrame();

    void setVSync();

    // variables
    Camera* cameraComponent = nullptr;
    Scene* currentScene = nullptr;
    std::vector<std::shared_ptr<Scene>> scenes;
    FMOD::Studio::System *fmodSystem = NULL;
    FMOD::System *fmodCoreSystem = NULL;
    FMOD::DSP *fmodDSP = NULL;

    //shaders
    std::unique_ptr<Shader> shader;

    GameObject *cameraGameObject = nullptr;
    GameObject *currentCameraGameObject = nullptr;
    Camera *currentCameraComponent = nullptr;

    float lastWindowWidth, lastWindowHeight;

    std::shared_ptr<Texture> m_LogoIcon;

    AnimationLibrary &m_AnimationLibrary = AnimationLibrary::GetInstance();

    ResourceManager &m_ResourceManager = ResourceManager::GetInstance();

    std::string preparedSceneLoad = "";

public:

    /// <summary>
    /// Constructor for the dBengine class.
    /// </summary>
    dBengine();

    /// <summary>
    /// Initializes the dBengine.
    /// </summary>
    /// <returns>Success of initialization (bool)</returns>
    bool Initialize();

    /// <summary>
    /// Runs the dBengine.
    /// </summary>
    /// <returns>Exit code (int)</returns>
    int Run();

    /// <summary>
    /// Loading a scene from a file path.
    /// </summary>
    /// <param name="filePath"></param>
    void LoadScene(std::string filePath);
    void SaveScene(std::string filePath);
    void LoadSceneAsGameObject(std::string filePath, GameObject *parentToAddTo);
    void SetupAfterLoad(Scene *loadedScene);
    void CheckSceneLoad();

    void LoadSceneAsync(std::string filePath);
    std::optional<std::shared_ptr<Scene>> LoadSceneTask(std::string filePath);

    void LoadSceneAsGameObjectAsync(std::string filePath, GameObject* parentToAddTo);
    std::optional<std::shared_ptr<Scene>> LoadSceneAsGameObjectTask(std::string filePath, GameObject* parentToAddTo);

    void NewScene(std::string filePath);

    void SetupCollisionTestScene();
    void SetupPlayer();
    
    void SetupNavigationTestScene();

    /// <summary>
    /// Sets the window mode to fullscreen or windowed.
    /// </summary>
    /// <param name="inFullscreen">Window mode (bool)</param>
    void SetWindowMode(bool inFullscreen);

    /// <summary>
    /// Sets the window to fullscreen mode.
    /// </summary>
    void WindowFullscreen();

    /// <summary>
    /// Sets the window to windowed mode.
    /// </summary>
    void WindowWindowed();

    /// <summary>
    /// Returns whether the window is in fullscreen mode or not.
    /// </summary>
    /// <returns>Window mode (bool)</returns>
    bool IsFullscreen();

    void useCameraSignalled(std::string uuid, bool withLerp);
    void useCamera(GameObject *cameraGO, bool withLerp = false);
    void returnToSceneCamera(bool withLerp = false);

    /// <summary>
    /// Singleton instance of the engine's settings.
    /// </summary>
    EngineSettings& Settings = EngineSettings::GetEngineSettings();

    std::string SceneToLoadOnStart = "";
};



#endif //DBENGINE_H
