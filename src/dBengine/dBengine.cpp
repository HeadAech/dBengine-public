//
//
// Created by Hubert Klonowski on 25/03/2025.
//

#include "dBengine.h"

#include <glad/glad.h>

#include <chrono>
#include "Components/LuaComponent/LuaComponent.h"
#include "Components/MeshInstance/MeshInstance.h"
#include "Signal/Signals.h"
#include "Components/AudioListener/AudioListener.h"
#include "Components/AudioSource/AudioSource.h"  

#include "Components/TextRenderer/TextRenderer.h"
#include "Components/Lights/DirectionalLight/DirectionalLight.h"
#include <Components/Lights/PointLight/PointLight.h>
#include <Components/Lights/SpotLight/SpotLight.h>

#include <Serializers/SceneSerializer.h>
#include <Serializers/MaterialSerializer.h>
#include <ResourceManager/ResourceManager.h>
#include <Components/Animator/Animator.h>
#include <Components/Particles/ParticleSystem.h>
#include <Helpers/TimerHelper/TimerHelper.h>
#include <Components/Tag/Tag.h>
#include "Components/Control/Button/Button.h"
#include <Components/Control/Text/Text.h>

#include <Components/CollisionShape/CollisionShape.h>
 #include "dBphysics/dBphysics.h"
#include <Components/PhysicsBody/PhysicsBody.h>
#include <Components/PlayerController/PlayerController.h>
#include <Components/ThirdPersonCamera/ThirdPersonCamera.h>
#include "dBrender/PostProcessing/Shake/CameraShake.h"
#include <future>


//Debug
#include "Components/Timer/Timer.h"

#include <Components/AISystem/AISystem.h>
#include <dBrender/PostProcessing/Fade/FadeTransition.h>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

const     char*   glsl_version     = "#version 410";

dBengine::dBengine() { 
    this->uuid = UUID::generateUUID();
    Signals::FileToScene.connect(this->uuid,[this](std::string filePath) { LoadSceneAsync(filePath); });
    Signals::FileToGameObject.connect(this->uuid, [this](std::string filePath, GameObject *gameObjectParent) {
        LoadSceneAsGameObject(filePath, gameObjectParent);
    });
    Signals::SceneToFile.connect(this->uuid, [this](std::string filePath) { SaveScene(filePath); });
    Signals::NewScene.connect(this->uuid, [this](std::string filePath) { NewScene(filePath); });
    Signals::Engine_SetWindowFullscreen.connect(this->uuid, [this](bool inFullscreen) { 
        SetWindowMode(inFullscreen); 
        });
    Signals::Engine_UseCamera.connect(this->uuid,
                                      [this](std::string uuid, bool withLerp) { useCameraSignalled(uuid, withLerp); });
    Signals::Engine_ReturnToSceneCamera.connect(this->uuid, [this](bool withLerp) { returnToSceneCamera(withLerp); });
    m_LogoIcon = m_ResourceManager.LoadTextureFromFile("res/textures/dBengine_logo_icon.png");

}

void dBengine::glfw_error_callback(int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
    EngineDebug::GetInstance().PrintError("Glfw Error " + std::to_string(error) + " : " + description);
}

void dBengine::mouseCallback(GLFWwindow *window, double xpos, double ypos) {
    // inputManager.ProcessMouseMovement(window, xpos, ypos);
    Input::GetInstance().ProcessMouseMovement(window, xpos, ypos);
}

void dBengine::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // EngineDebug::GetInstance().lastInputTime = std::chrono::high_resolution_clock::now();
    }
}

void dBengine::window_size_callback(GLFWwindow *window, int width, int height) { 
    //glViewport(0, 0, width, height);
    //FrameBuffer::GetInstance().RescaleFrameBuffer(width, height);
    //EngineDebug::GetInstance().PrintDebug("Window resized. New width: " + std::to_string(width) +
                                          //", height: " + std::to_string(height));

    if (height == 0 || width == 0) {
        return; // Skip when minimized
    }
    Signals::Camera_UpdateAspectRatio.emit(static_cast<float>((float)width / (float)height));
}

void dBengine::WindowFullscreen() {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
    Settings.windowInFullscreen = true;
}

void dBengine::WindowWindowed() {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);
    glfwSetWindowMonitor(window, nullptr, 50, 50, WINDOW_WIDTH, WINDOW_HEIGHT, mode->refreshRate);
    Settings.windowInFullscreen = false;
}


bool dBengine::Initialize() {
    debug.EnableFileLogging();

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        debug.GetInstance().PrintError("Failed to initalize GLFW!");
        return false;
    }

    // GL 4.6 + GLSL 460
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ 

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    try {
        Settings = EngineSettings::LoadFromFile(EngineSettings::SETTINGS_FILE);
        EngineDebug::GetInstance().PrintInfo("Loaded settings from file: " + EngineSettings::SETTINGS_FILE);
    } catch (const std::exception &e) {
        EngineDebug::GetInstance().PrintError("Failed to load settings: " + std::string(e.what()));
        // Set defaults if loading fails
    }

    // Create window with graphics context
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "dBengine v0.0.4", NULL, NULL);

    GLFWimage images[1];
    images[0].pixels = stbi_load("res/textures/dBengine_logo_icon.png", &images[0].width, &images[0].height, 0, 4);
    // set icon
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);

    if (window == NULL)
    {
        debug.GetInstance().PrintError("Failed to create GLFW Window!");
        return false;
    }

    if (!Settings.EditorEnabled) {
        SetWindowMode(true);
    } else {
        SetWindowMode(false);
    }

    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetFramebufferSizeCallback(window, window_size_callback);
    glfwWindowHint(GLFW_SAMPLES, 16);

    
    glfwSwapInterval(1); // Enable VSync - fixes FPS at the refresh rate of your screen
    Settings.SetVSync(true);

    bool err = !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    if (err)
    {
        debug.PrintError("Failed to initialize OpenGL loader");
        return false;
    }

    return true;
}


void dBengine::registerInputActions() {
    inputManager.RegisterAction("forward", GLFW_KEY_W);
    inputManager.RegisterAction("backward", GLFW_KEY_S);
    inputManager.RegisterAction("left", GLFW_KEY_A);
    inputManager.RegisterAction("right", GLFW_KEY_D);
    inputManager.RegisterAction("up_arrow", GLFW_KEY_UP);
    inputManager.RegisterAction("down_arrow", GLFW_KEY_DOWN);
    inputManager.RegisterAction("left_arrow", GLFW_KEY_LEFT);
    inputManager.RegisterAction("right_arrow", GLFW_KEY_RIGHT);
    inputManager.RegisterAction("jump", GLFW_KEY_SPACE);
    inputManager.RegisterAction("enter", GLFW_KEY_ENTER);
    inputManager.RegisterAction("left_shift", GLFW_KEY_LEFT_SHIFT);
    inputManager.RegisterAction("escape", GLFW_KEY_ESCAPE);
    inputManager.RegisterAction("mouse1", GLFW_MOUSE_BUTTON_LEFT, true);
    inputManager.RegisterAction("mouse2", GLFW_MOUSE_BUTTON_RIGHT, true);
    inputManager.RegisterAction("toggle_camera", GLFW_KEY_Z);
    inputManager.RegisterAction("attack", GLFW_KEY_F, false);
    inputManager.RegisterAction("TAB", GLFW_KEY_TAB);

    //editor actions
    inputManager.RegisterAction("gizmo_snapping", GLFW_KEY_LEFT_SHIFT);
    inputManager.RegisterAction("gizmo_scale", GLFW_KEY_S);
    inputManager.RegisterAction("gizmo_rotate", GLFW_KEY_R);
    inputManager.RegisterAction("gizmo_translate", GLFW_KEY_E);
    inputManager.RegisterAction("reload", GLFW_KEY_APOSTROPHE);

    //combos?
    inputManager.RegisterAction("c1", GLFW_KEY_B);
    inputManager.RegisterAction("c2", GLFW_KEY_N);
    inputManager.RegisterAction("c3", GLFW_KEY_M);

    inputManager.RegisterAction("test1", GLFW_KEY_K);
    inputManager.RegisterAction("test2", GLFW_KEY_L);
}

void dBengine::endFrame() {
    glfwPollEvents();
    glfwMakeContextCurrent(window);
    glfwSwapBuffers(window);
}

void dBengine::setVSync() {
    glfwSwapInterval(Settings.m_vsyncEnabled);
    // 1 - enable Vsync, 0 - disable Vsync
}

int dBengine::setup() {
    if (!Initialize())
    {
        debug.GetInstance().PrintError("Failed to initialize project!");
        return EXIT_FAILURE;
    }

    TimerHelper setupTimer("dBengine::setup");

    renderer.window = window;

    debug.GetInstance().PrintInfo("Initialized project.");

    engineGUI.Init(window, glsl_version);
    debug.GetInstance().PrintInfo("Initialized ImGui.");

    inputManager.SetWindow(window);
    Ref::GetInstance().InitializeFMOD();
        
    

    /// Bank load *important stuff*
    
    Ref::GetInstance().LoadFmodBank("res/audio/orpheus/Build/Desktop/Master.bank");
    Ref::GetInstance().LoadFmodBank("res/audio/orpheus/Build/Desktop/Master.strings.bank");
    Ref::GetInstance().LoadFmodBank("res/audio/orpheus/Build/Desktop/BossFight.bank");
    Ref::GetInstance().LoadFmodBank("res/audio/orpheus/Build/Desktop/BossSounds.bank");
    Ref::GetInstance().LoadFmodBank("res/audio/orpheus/Build/Desktop/SpeakerSounds.bank");
    Ref::GetInstance().LoadFmodBank("res/audio/orpheus/Build/Desktop/OrpheusSounds.bank");
    Ref::GetInstance().LoadFmodBank("res/audio/orpheus/Build/Desktop/UISounds.bank");
    Ref::GetInstance().LoadFmodBank("res/audio/orpheus/Build/Desktop/MainMenuTutorial.bank");
    
    Signals::EngineSettings_ChangedVSync.connect(this->uuid, [this] { setVSync(); });

    Signals::Camera_UpdateAspectRatio.connect(this->uuid, [this](float newAspectRatio) {
        /*shader->Use();
        shader->SetMat4("view", cameraComponent->GetViewMatrix());
        glm::mat4 proj = cameraComponent->GetProjectionMatrix();
        shader->SetMat4("projection", proj);*/
    });

    Signals::Engine_PrepareSceneLoad.connect(this->uuid, [this] (std::string path)
        {
            preparedSceneLoad = path;
        });


    // register keys in input manager
    registerInputActions();

    // debug info about hardware
    GLint maxVertices;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES, &maxVertices);
    std::cout << "Max vertices allowed by hardware: " << maxVertices << std::endl;

    GLint maxComponents;
    glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_COMPONENTS, &maxComponents);
    std::cout << "Max components allowed by hardware: " << maxComponents << std::endl;

    int maxTextures;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextures);
    std::cout << "Max fragment shader samplers: " << maxTextures << std::endl;

    // create shader program
    //shader = new Shader("res/shaders/basic.vert", "res/shaders/basic.frag");
    shader = std::make_unique<Shader>("res/shaders/blinn-phong/shader.vert", "res/shaders/blinn-phong/shader.frag");
    
    renderer.SetShader(shader.get());
    renderer.CreateShaderPrograms();
    
    scenes.push_back(std::move(std::make_unique<Scene>("Main Scene")));
    currentScene = scenes.back().get();
    currentScene->sceneCameraObject = std::make_unique<GameObject>("sceneCamera");
    currentCameraGameObject = currentScene->sceneCameraObject.get(); 
    currentCameraComponent = currentCameraGameObject->AddComponent<Camera>(defaultAspectRatio);
    returnToSceneCamera();
    engineGUI.SetCamera(currentCameraComponent);

    currentCameraGameObject->transform.SetLocalPosition({0.0f, 0.0f, -5.0f});
    currentCameraGameObject->AddComponent<LuaComponent>("res/scripts/camera/camera_controller.lua");

    GameObject *worldEnv = currentScene->CreateGameObject("World Environment");
    worldEnv->AddComponent<WorldEnvironment>("res/textures/hdri/skybox.hdr");

    GameObject* audioSource = currentScene->CreateGameObject("Audio Source");
    //audioSource->AddComponent<AudioSource>("res/audio/orpheus/Build/Desktop/BossFight.bank", FMOD_STUDIO_LOAD_BANK_NORMAL, true);
    //audioSource->GetComponent<AudioSource>()->Play("event:/BossFight");
    //audioSource->GetComponent<AudioSource>()->SetVolumeAll(0.1f);
    //SetupCollisionTestScene();
    //SetupPlayer();
    //SetupNavigationTestScene(); - not serialized yet.

    Signals::InstantiateGameObject.connect(this->uuid, [this](std::string name, std::string modelPath, glm::vec3 pos, GameObject* parent) {
        GameObject *instance = new GameObject(name);
        std::string newName = currentScene->CheckGameObjectNameSiblings(name, parent);
        instance->name = newName;
        instance->transform.SetLocalPosition(pos);
        Util::FixPathString(modelPath);

        instance->AddComponent<MeshInstance>();
        instance->GetComponent<MeshInstance>()->LoadModel(modelPath);
        instance->GetComponent<MeshInstance>()->m_modelPath = modelPath;
        //instance->GetComponent<MeshInstance>()->m_modelPath = modelPath;
        parent->AddChild(instance);
        engineGUI.selectedGameObject = parent->children.back().get();
    }
    );

    Signals::InstantiateEmptyGameObject.connect(this->uuid, [this](GameObject* parent) 
    { 
        GameObject *instance = new GameObject("Empty");
        std::string newName = currentScene->CheckGameObjectNameSiblings("Empty", parent);
        instance->name = newName;
        instance->transform.SetLocalPosition({0, 0, 0});
        parent->AddChild(instance);
        engineGUI.selectedGameObject = parent->children.back().get();
    });

    GameObject *dirLight = currentScene->CreateGameObject("Directional Light");
    dirLight->transform.SetEulerRotation({116, -31, 164});
    dirLight->AddComponent<DirectionalLight>();
    
    renderer.Init();
    engineGUI.SetScene(currentScene);
    renderer.m_activeScene = currentScene;

    Ref::CurrentScene = currentScene;

    Signals::PostProcessing_TriggerCameraShake.connect(this->uuid, [this](float intensity, float duration) {
        CameraShake::GetInstance().TriggerShake(intensity, duration);
    });

    // working place, maybe find a better place for this
    //SceneToLoadOnStart = "res/scenes/tut-trig.scene";
    //SceneToLoadOnStart = "res/scenes/ArenaLevel2.scene";

    if (!SceneToLoadOnStart.empty())
    {
        LoadScene(SceneToLoadOnStart);
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        if (!Settings.EditorEnabled)
        {
            Ref::AspectRatio = static_cast<float>((float)width / (float)height);

            Signals::Camera_UpdateAspectRatio.emit(Ref::AspectRatio);
            renderer.framebufferMultisample.RescaleFrameBuffer(width, height);
            renderer.framebuffer.RescaleFrameBuffer(width, height);
            renderer.renderFrameBuffer.RescaleFrameBuffer(width, height);
            renderer.bloomRenderer.Rescale(width, height);
            renderer.HUDBloomRenderer.Rescale(width, height);
            renderer.UIFramebuffer.RescaleFrameBuffer(width, height);
        }
    }

    //Settings.EditorEnabled = false;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

int dBengine::Run() {
    setup();
    debug.PrintInfo("Engine Started...");

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;
        // auto currentFrameTime = std::chrono::high_resolution_clock::now();
        // debug.lastFrameTime = currentFrameTime;

        //debug
        // if (debug.lastInputTime.time_since_epoch().count() > 0) {
        //     debug.inputLatency = std::chrono::duration<double, std::milli>(debug.lastFrameTime - debug.lastInputTime).count();
        // }

        // Process I/O operations here
        input();

        // Update game objects' state here
        update();

        // OpenGL rendering code here
        render();
        
        // End frame and swap buffers (double buffering)
        endFrame();
        
    }

    // Cleanup
    engineGUI.Shutdown();

    glfwDestroyWindow(window);
    glfwTerminate();

    debug.PrintInfo("Engine Shutting down...");
    return 0;
}


void dBengine::input() {
    inputManager.ProcessInput(window);
    if (inputManager.IsActionJustPressed("escape")) {
        //glfwSetWindowShouldClose(window, true);
        Signals::Editor_ExitFullscreen.emit();
        FadeTransition::GetInstance().Start([this]()
            {
                std::cout << "Done fading" << std::endl;
                LoadSceneAsync("res/scenes/Other/main-menu.scene");
            });
    }

    if (inputManager.IsActionJustPressed("reload")) {
        Signals::ReloadShaders.emit();
    }

    // alt enter
    if (HOLD_ALT && inputManager.IsActionJustPressed("enter")) {
        if (Settings.windowInFullscreen) {
            WindowWindowed();
        } else {
            WindowFullscreen();
        }
    }
}

void dBengine::update() {
    if (!preparedSceneLoad.empty())
    {
        LoadSceneAsync(preparedSceneLoad);
        //LoadScene(preparedSceneLoad);
        preparedSceneLoad.clear();
    }

    //debug
    auto start = std::chrono::high_resolution_clock::now();

    m_ResourceManager.ProcessPendingModels();
    m_ResourceManager.ProcessPendingTextures();

    //
    int width, height;
    glfwGetWindowSize(window, &width, &height);

    if (width != 0 && height != 0)
    {
        Ref::WindowSize = { width,  height };
        if (width != lastWindowWidth || height != lastWindowHeight)
        {

            glm::mat4 projection = currentCameraComponent->GetProjectionMatrix();
            shader->Use();
            shader->SetMat4("projection", projection);
            std::shared_ptr<Shader> particlesShader = renderer.GetParticlesShader();
            particlesShader->Use();
            particlesShader->SetMat4("projection", projection);
            lastWindowWidth = width;
            lastWindowHeight = height;
            if (!Settings.EditorEnabled)
            {
                Ref::AspectRatio = static_cast<float>((float)width / (float)height);

                Signals::Camera_UpdateAspectRatio.emit(Ref::AspectRatio);
                renderer.framebufferMultisample.RescaleFrameBuffer(width, height);
                renderer.framebuffer.RescaleFrameBuffer(width, height);
                renderer.renderFrameBuffer.RescaleFrameBuffer(width, height);
                renderer.bloomRenderer.Rescale(width, height);
                renderer.HUDBloomRenderer.Rescale(width, height);
                renderer.UIFramebuffer.RescaleFrameBuffer(width, height);
            }
            
        }
    }
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    if (!Settings.EditorEnabled)
    {
        Ref::MousePosition = { (float)mouseX, (float)mouseY };
    }

    

    if (Settings.m_inPlayMode)
    {
        physics.Update(deltaTime);
    }
    if (Settings.EditorEnabled)
    {
        engineGUI.Update(deltaTime);
    }

    currentScene->CommitPendingGameObjects();
    currentScene->Update(deltaTime);
    
    fmodSystem->update();

    // post processing - shake
    auto& cameraShake = CameraShake::GetInstance();

    cameraShake.Update(deltaTime);
    cameraShake.SetTime(glfwGetTime());

    // update transition effect
    FadeTransition::GetInstance().Update(deltaTime);

    //debug
    auto end = std::chrono::high_resolution_clock::now();
    debug.updateTime = std::chrono::duration<float, std::milli>(end - start).count();
    //

    m_Scheduler.ExecuteTasks();
    CheckSceneLoad();
}

void dBengine::render() {
    renderer.Render(currentCameraComponent);
    // Draw ImGui
    //glDisable(GL_MULTISAMPLE);
    if (Settings.EditorEnabled)
    {
        engineGUI.Render();
    }
    //glEnable(GL_MULTISAMPLE);
}


void dBengine::SaveScene(std::string filePath) {
    if (Util::fileExists(filePath)) {
        //OVERWRITE WISH STUFF
    }
    if (sceneSerializer.Serialize(currentScene, filePath)) {

        debug.PrintInfo("The scene, entitled '" + currentScene->name + "', hath been preserved to the file '" +
                        filePath + "'");
    } else {
        debug.PrintInfo("U forgot again to update serialization stuff '" + currentScene->name + "', hath been NOT saved...");
    }
}

void dBengine::LoadScene(std::string filePath) { 
    if (!Util::fileExists(filePath)){
        debug.PrintError("My sincerest apologies, but it appears the file located at '" + filePath +
                         "' could not be found");
        return;
    }
        CollisionShape::colliders.clear(); 

        currentScene->isActive = false;
        renderer.prepForSceneLoad();

        std::string sceneName = filePath;
        size_t lastSlash = sceneName.find_last_of("\\/");

        size_t lastDot = sceneName.find_last_of('.');

        if (lastDot != std::string::npos && (lastSlash == std::string::npos || lastDot > lastSlash)) {
            sceneName = sceneName.substr(lastSlash == std::string::npos ? 0 : lastSlash + 1,
                                         lastDot - (lastSlash == std::string::npos ? 0 : lastSlash + 1));
        } else {
            sceneName = sceneName.substr(lastSlash == std::string::npos ? 0 : lastSlash + 1);
        }
        std::shared_ptr<Scene> newScene = std::make_shared<Scene>("loadedSceneFiller");
        newScene->isActive = true;
        if (!sceneSerializer.DeserializeToScene(newScene.get(), filePath)) {
            debug.PrintError("My sincerest apologies, but it appears I cannot load the file located at '" + filePath +
                             "'");
            currentScene->isActive = true;
            return;
        }
        auto it = std::find_if(scenes.begin(), scenes.end(),
                               [&](const std::shared_ptr<Scene> &s) { return s->name == sceneName; });
        if (it != scenes.end()) {
            scenes.erase(it);
        }
        currentScene = nullptr;
        engineGUI.selectedGameObject = nullptr;


        bool savedFlag = false;
        for (auto &scene: scenes) {
            if (scene->name == newScene->name) {
                scene.reset();
                scene = std::move(newScene);
                currentScene = scene.get();
                savedFlag = true;
                break;
            }
        }
        if (!savedFlag) {
            scenes.push_back(std::move(newScene));
            currentScene = scenes.back().get();
        }
        renderer.m_activeScene = currentScene;
        engineGUI.SetScene(currentScene);
        engineGUI.SetCamera(currentCameraComponent);

        Ref::CurrentScene = currentScene;

        SetupAfterLoad(currentScene);
        debug.PrintInfo("File: '" + filePath + "' loaded correctly to scene");
}


void dBengine::LoadSceneAsGameObject(std::string filePath, GameObject* parentToAddTo) {
    if (!Util::fileExists(filePath)) {
        debug.PrintError("My sincerest apologies, but it appears the file located at '" + filePath +
                         "' could not be found");
        return;
    }
    std::string sceneName = filePath;
    size_t lastSlash = sceneName.find_last_of("\\/");

    size_t lastDot = sceneName.find_last_of('.');

    if (lastDot != std::string::npos && (lastSlash == std::string::npos || lastDot > lastSlash)) {
        sceneName = sceneName.substr(lastSlash == std::string::npos ? 0 : lastSlash + 1,
                                     lastDot - (lastSlash == std::string::npos ? 0 : lastSlash + 1));
    } else {
        sceneName = sceneName.substr(lastSlash == std::string::npos ? 0 : lastSlash + 1);
    }
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>("loadedSceneFiller");

    if (!sceneSerializer.DeserializeToMap(currentScene, filePath)) {

        debug.PrintError("My sincerest apologies, but it appears I cannot load the file located at '" + filePath + "'");
        return;
    }
    if (!sceneSerializer.createGameObjectFromScene(currentScene, filePath, parentToAddTo, true)) {
        debug.PrintError("My sincerest apologies, but it appears I cannot createGameObjectFromScene from the file located at '" + filePath + "'");
        return;
    }
    //renderer.flattenHierarchy(parentToAddTo->children.back().get());
    auto it = std::find_if(scenes.begin(), scenes.end(),
                           [&](const std::shared_ptr<Scene> &s) { return s->name == sceneName; });

    if (it != scenes.end()) {
        scenes.erase(it);
    }


    bool savedFlag = false;
    for (auto &scene: scenes) { //chyba jej nie musze podmieniac, tylko se sprawdze czy modify time byl inny i git.
        if (scene->name == newScene->name) {
            //scene.reset();
            //scene = std::move(newScene);
            //scene->sceneRootObject->isScene = true;
            //savedFlag = true;
            //parentToAddTo->AddChild(scene->sceneRootObject.get());
            //currentScene->transferVectorsFromScene(scene.get());
            //scene->isActive = false;
            break;
        }
    }
    if (!savedFlag) {
        /*scenes.push_back(std::move(newScene));
        scenes.back()->sceneRootObject->isScene = true;
        currentScene->transferVectorsFromScene(scenes.back().get());
        scenes.back().get()->isActive = false;*/
    }
    engineGUI.selectedGameObject = parentToAddTo->children.back().get();


    debug.PrintInfo("File: '" + filePath + "' loaded correctly to gameObject: '" + parentToAddTo->name + "'");
}

void dBengine::LoadSceneAsGameObjectAsync(std::string filePath, GameObject* parentToAddTo)
{
    std::lock_guard<std::mutex> lock(sceneMutex);

    pendingSceneFutures.emplace_back(
        std::async(std::launch::async, &dBengine::LoadSceneAsGameObjectTask, this, filePath, parentToAddTo)
    );
}

std::optional<std::shared_ptr<Scene>> dBengine::LoadSceneAsGameObjectTask(std::string filePath, GameObject* parentToAddTo)
{
    if (!Util::fileExists(filePath))
    {
        debug.PrintError("File not found: " + filePath);
        return std::nullopt;
    }

    std::string sceneName = filePath;
    size_t lastSlash = sceneName.find_last_of("\\/");
    size_t lastDot = sceneName.find_last_of('.');

    if (lastDot != std::string::npos && (lastSlash == std::string::npos || lastDot > lastSlash))
    {
        sceneName = sceneName.substr(lastSlash == std::string::npos ? 0 : lastSlash + 1, lastDot - (lastSlash == std::string::npos ? 0 : lastSlash + 1));
    }
    else
    {
        sceneName = sceneName.substr(lastSlash == std::string::npos ? 0 : lastSlash + 1);
    }

    std::shared_ptr<Scene> newScene = std::make_shared<Scene>("loadedSceneFiller");

    if (!sceneSerializer.DeserializeToMap(currentScene, filePath))
    {
        debug.PrintError("Cannot load file: " + filePath);
        return std::nullopt;
    }

    if (!sceneSerializer.createGameObjectFromScene(currentScene, filePath, parentToAddTo, true))
    {
        debug.PrintError("Cannot create GameObject from file: " + filePath);
        return std::nullopt;
    }

    auto it = std::find_if(scenes.begin(), scenes.end(),
        [&](const std::shared_ptr<Scene>& s)
        {
            return s->name == sceneName;
        });

    if (it != scenes.end())
    {
        scenes.erase(it);
    }

    engineGUI.selectedGameObject = parentToAddTo->children.back().get();
    debug.PrintInfo("File: '" + filePath + "' loaded correctly to gameObject: '" + parentToAddTo->name + "'");
    Signals::ForceRenderLights.emit();
    return newScene;
}

void dBengine::SetupAfterLoad(Scene* loadedScene) {
     
    currentCameraGameObject = loadedScene->sceneCameraObject.get();
    if (currentCameraGameObject) {
        currentCameraComponent = currentCameraGameObject->GetComponent<Camera>();
        returnToSceneCamera();
        engineGUI.SetCamera(currentCameraComponent);
        renderer.PrepareBuffersAfterLoad();
        GameObject *tppcam = currentScene->GetGameObject("TPP Camera");
        if (tppcam) {
            currentScene->sceneCameraObject->GetComponent<Camera>()->isUsed = false;
            Camera *tppcomp = tppcam->GetComponent<Camera>();
            tppcomp->isUsed = true;
            currentCameraComponent = tppcomp;
            currentCameraGameObject = tppcam;
            engineGUI.SetCamera(currentCameraComponent);
        }
        
        for (int i = 0; i < renderer.m_flatGameObjects.size(); i++) {
            for (int j = 0; j < renderer.m_flatGameObjects.at(i)->components.size(); j++) {
                if (auto *comp = dynamic_cast<NavigationMesh *>(renderer.m_flatGameObjects.at(i)->components.at(j).get())) {
                    comp->ResolveReferences(currentScene);
                }
                if (auto *comp = dynamic_cast<AISystem*>(renderer.m_flatGameObjects.at(i)->components.at(j).get())) {
                    comp->ResolveReferences(currentScene);
                }
            }
        }
        
        physics.Initialize();


    }
    else {
        EngineDebug::GetInstance().PrintError("Bro, get the camera - No camera found to view the scene");
        return;
    }
}

void dBengine::CheckSceneLoad()
{
    if (pendingSceneFuture.valid() && pendingSceneFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
    {
        std::optional<std::shared_ptr<Scene>> result;

        try
        {
            result = pendingSceneFuture.get();
        }
        catch (const sol::error& e)
        {
            debug.PrintError(std::string("Lua error while loading scene: ") + e.what());
            NewScene("New Scene");
            return;
        }
        catch (const std::exception& e)
        {
            debug.PrintError(std::string("Exception while loading scene: ") + e.what());
            NewScene("New Scene");
            return;
        }
        catch (...)
        {
            debug.PrintError("Unknown error while loading scene.");
            NewScene("New Scene");
            return;
        }


        //engineGUI.HideLoadingScreen();

        if (!result)
        {
            debug.PrintError("My sincerest apologies, but it appears I cannot load the scene.");
            NewScene("New Scene");
            return;
        }

        auto newScene = result.value();

        auto it = std::find_if(scenes.begin(), scenes.end(),
                               [&](const std::shared_ptr<Scene> &s) { return s->name == currentScene->name; });

        if (it != scenes.end()) {
            scenes.erase(it);
        }

        currentScene = nullptr;
        engineGUI.selectedGameObject = nullptr;

        bool savedFlag = false;
        for (auto& scene : scenes)
        {
            if (scene->name == newScene->name)
            {
                scene = newScene;
                currentScene = scene.get();
                savedFlag = true;
                break;
            }
        }

        if (!savedFlag)
        {
            scenes.push_back(newScene);
            currentScene = scenes.back().get();
        }

        renderer.m_activeScene = currentScene;
        engineGUI.SetScene(currentScene);
        engineGUI.SetCamera(currentCameraComponent);
        Ref::CurrentScene = currentScene;

        SetupAfterLoad(currentScene);
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        if (!Settings.EditorEnabled)
        {
            Ref::AspectRatio = static_cast<float>((float)width / (float)height);

            Signals::Camera_UpdateAspectRatio.emit(Ref::AspectRatio);
            renderer.framebufferMultisample.RescaleFrameBuffer(width, height);
            renderer.framebuffer.RescaleFrameBuffer(width, height);
            renderer.renderFrameBuffer.RescaleFrameBuffer(width, height);
            renderer.bloomRenderer.Rescale(width, height);
            renderer.HUDBloomRenderer.Rescale(width, height);
            renderer.UIFramebuffer.RescaleFrameBuffer(width, height);
        }
        debug.PrintInfo("File loaded correctly to scene.");
        Ref::SceneLoading = false;
        Signals::ForceRenderLights.emit();
    }
}

void dBengine::LoadSceneAsync(std::string filePath)
{
    if (!Util::fileExists(filePath.data()))
    {
        //debug.PrintError("My sincerest apologies, but it appears the file located at '" + filePath + "' could not be found");
        return;
    }

    currentScene->isActive = false;
    renderer.prepForSceneLoad();
    //engineGUI.ShowLoadingScreen();

    // Start background loading
    Ref::SceneLoading = true;
    pendingSceneFuture = std::async(std::launch::async, &dBengine::LoadSceneTask, this, filePath);
}

std::optional<std::shared_ptr<Scene>> dBengine::LoadSceneTask(std::string filePath)
{
    std::string sceneName = filePath.data();
    size_t lastSlash = sceneName.find_last_of("\\/");
    size_t lastDot = sceneName.find_last_of('.');

    if (lastDot != std::string::npos && (lastSlash == std::string::npos || lastDot > lastSlash))
    {
        sceneName = sceneName.substr(lastSlash == std::string::npos ? 0 : lastSlash + 1,
            lastDot - (lastSlash == std::string::npos ? 0 : lastSlash + 1));
    }
    else
    {
        sceneName = sceneName.substr(lastSlash == std::string::npos ? 0 : lastSlash + 1);
    }

    auto newScene = std::make_shared<Scene>(sceneName);
    newScene->isActive = true;

    if (!sceneSerializer.DeserializeToScene(newScene.get(), filePath.data()))
    {
        return std::nullopt;
    }

    return newScene;

}

void dBengine::NewScene(std::string filePath) { 
    
    auto it = std::find_if(scenes.begin(), scenes.end(),
                           [&](const std::shared_ptr<Scene> &s) { return s->name == currentScene->name; });

    if (it != scenes.end()) {
        scenes.erase(it);
    }
    currentScene = nullptr;
    renderer.prepForSceneLoad();
    scenes.push_back(std::make_unique<Scene>("New Scene")); 
    currentScene = scenes.back().get();
    currentScene->sceneCameraObject = std::make_unique<GameObject>("sceneCamera");
    currentCameraGameObject = currentScene->sceneCameraObject.get();
    currentCameraComponent = currentCameraGameObject->AddComponent<Camera>(defaultAspectRatio);
    currentCameraGameObject->transform.SetLocalPosition({-5.0f, -5.0f, -5.0f});
    currentCameraGameObject->AddComponent<LuaComponent>("res/scripts/camera/camera_controller.lua");
    returnToSceneCamera();
    engineGUI.SetCamera(currentCameraComponent);
    engineGUI.selectedGameObject = nullptr;
    Ref::CurrentScene = currentScene;
    renderer.m_activeScene = currentScene;
    engineGUI.SetScene(currentScene);
    physics.Initialize();
    if (currentCameraGameObject) {
        renderer.PrepareBuffersAfterLoad();
    }
}

void dBengine::SetupCollisionTestScene() {
    //GameObject *platform2 = currentScene->CreateGameObject("Blockout");
    //platform2->transform.SetLocalPosition({0.0f, -5.0f, 0.0f});
    //platform2->AddComponent<MeshInstance>()->LoadModel("res/models/blockout/blockout2.obj");
    //platform2->transform.SetScale({10.0f, 10.0f, 10.0f});
    //auto collider_p = platform2->AddComponent<CollisionShape>();
    //collider_p->SetShapeType(ShapeType::BOX);
    //collider_p->SetBoxSize({10.0f, 1.9f, 10.0f});
    //auto rigidBodyPlatform = platform2->AddComponent<PhysicsBody>();
    //rigidBodyPlatform->SetStatic(true);
    //collider_p->SetCollisionLayer(1 << 0);
    //collider_p->SetCollisionMask(1 << 1 | 1 << 2);

    //// player for testing
    //GameObject *player = currentScene->CreateGameObject("Player");
    //player->transform.SetLocalPosition({0.0f, 10.0f, 5.0f});
    ////player->transform.SetScale({1.0f, 2.0f, 1.0f});
    //auto playerMesh = player->AddComponent<MeshInstance>();
    //playerMesh->LoadModel("res/models/capsule/capsule.obj");

    //auto playerCollider = player->AddComponent<CollisionShape>();
    ////playerCollider->SetBoxSize({1.0f, 2.0f, 1.0f});
    //playerCollider->SetShapeType(ShapeType::CAPSULE);
    //playerCollider->SetCapsuleParams(1.0f, 4.0f);
    //auto playerRigidBody = player->AddComponent<PhysicsBody>();

    //player->AddComponent<LuaComponent>("res/scripts/player_controller.lua");

    //GameObject *player2 = currentScene->CreateGameObject("Player2");
    //player2->transform.SetLocalPosition({3.0f, 4.0f, 5.0f});
    //auto playerMesh2 = player2->AddComponent<MeshInstance>();
    //playerMesh2->LoadModel("res/models/capsule/capsule.obj");
    // auto playerCollider2 = player2->AddComponent<CollisionShape>();
    //playerCollider2->SetShapeType(ShapeType::CAPSULE);
    //playerCollider2->SetCapsuleParams(1.0f, 4.0f);
    //auto playerRigidBody2 = player2->AddComponent<PhysicsBody>();
    //playerRigidBody2->SetMass(1.0f);

    // for (int i = 0; i < 4; i++) {
    //    GameObject *box = currentScene->CreateGameObject("Box_" + std::to_string(i));
    //    box->transform.SetLocalPosition({-20.0f + i * 3, 7.0, 0.0f + i%10 * 3});
    //    box->AddComponent<MeshInstance>()->LoadModel("res/models/cube/cube.obj");
    //    box->AddComponent<CollisionShape>("OnAreaEntered")->SetBoxSize({2.0f, 2.0f, 2.0f});
    //    //box->GetComponent<CollisionShape>()->SetIsCollisionArea(true);
    //    auto rigidBody = box->AddComponent<PhysicsBody>();
    //    //rigidBody->SetStatic(true);
    //    box->GetComponent<CollisionShape>()->SetCollisionLayer(1 << 2);
    //    box->GetComponent<CollisionShape>()->SetCollisionMask(1 << 0);
    //}

    //player2->AddComponent<LuaComponent>("res/scripts/enemy.lua");

    //GameObject *player3 = currentScene->CreateGameObject("Player3");
    //player3->transform.SetLocalPosition({6.0f, 4.0f, 5.0f});
    ////player3->transform.SetScale({1.0f, 2.0f, 1.0f});
    //auto playerMesh3 = player3->AddComponent<MeshInstance>();
    //playerMesh3->LoadModel("res/models/capsule/capsule.obj");

    //////player3->AddComponent<LuaComponent>("res/scripts/enemy.lua");

    //auto playerCollider3 = player3->AddComponent<CollisionShape>();
    ////playerCollider3->SetBoxSize({2.0f, 4.0f, 2.0f});
    //playerCollider3->SetShapeType(ShapeType::CAPSULE);
    //playerCollider3->SetCapsuleParams(1.0f, 4.0f);
    //auto playerRigidBody3 = player3->AddComponent<PhysicsBody>();
    //playerRigidBody3->SetMass(1.0f);
    //playerCollider3->SetCollisionLayer(1 << 2);
    //playerCollider3->SetCollisionMask(1 << 0); 

}

void dBengine::SetupPlayer() {
    GameObject *player = currentScene->CreateGameObject("Player");
    player->transform.SetLocalPosition({-10.0f, 2.0f, 5.0f});
    auto playerMesh = player->AddComponent<MeshInstance>();
    playerMesh->LoadModel("res/models/skeleton/skeleton1.fbx");
    player->transform.SetScale({0.04f, 0.04f, 0.04f});

    auto playerPhysics = player->AddComponent<PhysicsBody>();
    playerPhysics->SetMass(1.0f); 
    playerPhysics->useGravity = false;
    playerPhysics->restitution = 0.1f; 
    playerPhysics->linearDamping = 0.1f; 

    auto playerCollider = player->AddComponent<CollisionShape>();
    playerCollider->SetShapeType(ShapeType::CAPSULE);

    playerCollider->SetCapsuleParams(30.0, 190.0);
    playerCollider->SetPositionOffset(glm::vec3(0.0f, 3.3f, 0.0f)); 

    playerCollider->SetCollisionLayer(1 << 1);
    playerCollider->SetCollisionMask(1 << 0);


    auto controller = player->AddComponent<PlayerController>();

    GameObject *tppCamera = currentScene->CreateGameObject("TPP Camera");
    tppCamera->transform.SetLocalPosition({3.0f, 4.0f, 8.0f});
    tppCamera->AddComponent<LuaComponent>("res/scripts/camera/tpp_camera_controller.lua");
    auto tppCameraComponent = tppCamera->AddComponent<ThirdPersonCamera>(defaultAspectRatio);
    tppCameraComponent->SetTarget(player);
    tppCameraComponent->orbitDistance = 20.0f;
    tppCameraComponent->orbitYaw = 45.0f;
    tppCameraComponent->orbitPitch = 25.0f;
    tppCameraComponent->targetHeightOffset = 10.0f;
    

    Animator *animator = player->AddComponent<Animator>();
    AnimationLibrary &animLib = m_AnimationLibrary;
    animLib.AddAnimation("idle", "res/animations/idle.fbx");
    animLib.AddAnimation("run", "res/animations/run.fbx");
    animLib.AddAnimation("attack", "res/animations/attack_1.fbx");

    auto idleAnim = animLib.GetAnimation("idle");
    auto runAnim = animLib.GetAnimation("run");
    auto attackAnim = animLib.GetAnimation("attack");

    //animator->AddAnimation(idleAnim);
    //animator->AddAnimation(runAnim);
    //animator->AddAnimation(attackAnim);


    GameObject *hitboxObj = new GameObject("AttackHitbox");
    player->AddChild(hitboxObj);
    hitboxObj->transform.SetLocalPosition({0.0f, 0.0f, 60.5f});
    Hitbox *hitbox = hitboxObj->AddComponent<Hitbox>("PlayerAttack");
    hitbox->SetShapeType(ShapeType::BOX);
    hitbox->SetBoxSize({3.0f, 6.0f, 3.0f});
    hitbox->SetVisible(true);
    hitbox->SetColor({1.0f, 0.0f, 0.0f});
    hitbox->SetValidTargetTags({"Enemy"});
    hitbox->SetIsCollisionArea(false);

    //GameObject *monster = currentScene->CreateGameObject("Enemy");
    //monster->AddComponent<LuaComponent>("res/scripts/enemy.lua");
    //monster->AddComponent<Tag>("Enemy");
    //monster->transform.SetLocalPosition({0.0f, 15.0f, 10.0f});
    //auto monsterMesh = monster->AddComponent<MeshInstance>();
    //monsterMesh->LoadModel("res/models/orpheus/orpheus.fbx");

    //auto monsterPhysics = monster->AddComponent<PhysicsBody>();
    //monsterPhysics->SetMass(1.0f);
    //monsterPhysics->useGravity = true;
    //monsterPhysics->restitution = 0.1f;
    //monsterPhysics->linearDamping = 0.1f;

    //auto monsterColl = monster->AddComponent<CollisionShape>();
    //monsterColl->SetShapeType(ShapeType::CAPSULE);
    //monsterColl->SetCapsuleParams(2.0f, 10.0f);
}

bool dBengine::IsFullscreen() { return Settings.windowInFullscreen; }

void dBengine::useCameraSignalled(std::string uuid, bool withLerp){
    useCamera(currentScene->GetGameObjectUUID(uuid),withLerp);
}

void dBengine::useCamera(GameObject* cameraGO, bool withLerp) {
    currentCameraComponent->isUsed = false;
    cameraGO->GetComponent<Camera>()->isUsed = true;
    if (withLerp) {
        cameraGO->GetComponent<Camera>()->AssignLerpValues(currentCameraComponent);
    } 
    currentCameraGameObject = cameraGO;
    currentCameraComponent = currentCameraGameObject->GetComponent<Camera>();
    engineGUI.SetCamera(currentCameraComponent);

}

void dBengine::returnToSceneCamera(bool withLerp) {
    currentCameraComponent->isUsed = false;
    if (withLerp) {
        currentScene->sceneCameraObject.get()->GetComponent<Camera>()->AssignLerpValues(currentCameraComponent);
    }
    currentCameraGameObject = currentScene->sceneCameraObject.get();
    currentCameraComponent = currentCameraGameObject->GetComponent<Camera>();
    currentCameraGameObject->GetComponent<Camera>()->isUsed = true;
    engineGUI.SetCamera(currentCameraComponent);

}

void dBengine::SetWindowMode(bool inFullscreen) {

    if (inFullscreen) {
        WindowFullscreen();
    } else {
        WindowWindowed();
    }

}

void dBengine::SetupNavigationTestScene() {
    GameObject *plat5 = currentScene->CreateGameObject("Plat5 NAV TEST");
    plat5->AddComponent<MeshInstance>();
    plat5->GetComponent<MeshInstance>()->LoadModel("res/models/cube/cube.obj");
    plat5->transform.SetLocalPosition({0.0f, 80.0f, 0.0f});
    plat5->transform.SetScale({50.0f, 1.0f, 50.0f});
    auto collider_p5 = plat5->AddComponent<CollisionShape>();
    collider_p5->SetShapeType(ShapeType::BOX);
    collider_p5->SetBoxSize({2.0f, 2.0f, 2.0f});
    auto rigidBodyPlatform5 = plat5->AddComponent<PhysicsBody>();
    rigidBodyPlatform5->SetStatic(true);

    GameObject *plat6 = currentScene->CreateGameObject("Plat6 NAV TEST");
    plat6->AddComponent<MeshInstance>();
    plat6->GetComponent<MeshInstance>()->LoadModel("res/models/cube/cube.obj");
    plat6->transform.SetLocalPosition({1.8f, 86.9f, 24.4f});
    plat6->transform.SetScale({11.80f, 5.9f, 10.0f});
    auto collider_p6 = plat6->AddComponent<CollisionShape>();
    collider_p6->SetShapeType(ShapeType::BOX);
    collider_p6->SetBoxSize({2.0f, 2.0f, 2.0f});
    auto rigidBodyPlatform6 = plat6->AddComponent<PhysicsBody>();
    rigidBodyPlatform6->SetStatic(true);

    GameObject *plat7 = currentScene->CreateGameObject("Plat7 NAV TEST");
    plat7->AddComponent<MeshInstance>();
    plat7->GetComponent<MeshInstance>()->LoadModel("res/models/cube/cube.obj");
    plat7->transform.SetLocalPosition({30.8f, 83.5f, -8.2f});
    plat7->transform.SetScale({2.5f, 2.5f, 2.5f});
    auto collider_p7 = plat7->AddComponent<CollisionShape>();
    collider_p7->SetShapeType(ShapeType::BOX);
    collider_p7->SetBoxSize({2.0f, 2.0f, 2.0f});
    auto rigidBodyPlatform7 = plat7->AddComponent<PhysicsBody>();
    rigidBodyPlatform7->SetStatic(true);

    GameObject *plat8 = currentScene->CreateGameObject("Plat8 NAV TEST");
    plat8->AddComponent<MeshInstance>();
    plat8->GetComponent<MeshInstance>()->LoadModel("res/models/cube/cube.obj");
    plat8->transform.SetLocalPosition({-19.19f, 84.0f, -26.18f});
    plat8->transform.SetScale({10.0f, 3.0f, 10.0f});
    auto collider_p8 = plat8->AddComponent<CollisionShape>();
    collider_p8->SetShapeType(ShapeType::BOX);
    collider_p8->SetBoxSize({2.0f, 2.0f, 2.0f});
    auto rigidBodyPlatform8 = plat8->AddComponent<PhysicsBody>();
    rigidBodyPlatform8->SetStatic(true);

    GameObject *wall1 = currentScene->CreateGameObject("Wall1 NAV TEST");
    wall1->AddComponent<MeshInstance>();
    wall1->GetComponent<MeshInstance>()->LoadModel("res/models/cube/cube.obj");
    wall1->transform.SetLocalPosition({0.0f, 84.0f, 51.15f});
    wall1->transform.SetScale({50.0f, 3.0f, 1.0f});
    auto collider_wall1 = wall1->AddComponent<CollisionShape>();
    collider_wall1->SetShapeType(ShapeType::BOX);
    collider_wall1->SetBoxSize({2.0f, 2.0f, 2.0f});
    auto rigidBodyWall1 = wall1->AddComponent<PhysicsBody>();
    rigidBodyWall1->SetStatic(true);

    GameObject *wall2 = currentScene->CreateGameObject("Wall2 NAV TEST");
    wall2->AddComponent<MeshInstance>();
    wall2->GetComponent<MeshInstance>()->LoadModel("res/models/cube/cube.obj");
    wall2->transform.SetLocalPosition({0.0f, 84.0f, -51.15f});
    wall2->transform.SetScale({50.0f, 3.0f, 1.0f});
    auto collider_wall2 = wall2->AddComponent<CollisionShape>();
    collider_wall2->SetShapeType(ShapeType::BOX);
    collider_wall2->SetBoxSize({2.0f, 2.0f, 2.0f});
    auto rigidBodyWall2 = wall2->AddComponent<PhysicsBody>();
    rigidBodyWall2->SetStatic(true);

    GameObject *wall3 = currentScene->CreateGameObject("Wall3 NAV TEST");
    wall3->AddComponent<MeshInstance>();
    wall3->GetComponent<MeshInstance>()->LoadModel("res/models/cube/cube.obj");
    wall3->transform.SetLocalPosition({-51.15f, 84.0f, 0.0f});
    wall3->transform.SetScale({1.0f, 3.0f, 50.0f});
    auto collider_wall3 = wall3->AddComponent<CollisionShape>();
    collider_wall3->SetShapeType(ShapeType::BOX);
    collider_wall3->SetBoxSize({2.0f, 2.0f, 2.0f});
    auto rigidBodyWall3 = wall3->AddComponent<PhysicsBody>();
    rigidBodyWall3->SetStatic(true);

    GameObject *wall4 = currentScene->CreateGameObject("Wall4 NAV TEST");
    wall4->AddComponent<MeshInstance>();
    wall4->GetComponent<MeshInstance>()->LoadModel("res/models/cube/cube.obj");
    wall4->transform.SetLocalPosition({51.15f, 84.0f, 0.0f});
    wall4->transform.SetScale({1.0f, 3.0f, 50.0f});
    auto collider_wall4 = wall4->AddComponent<CollisionShape>();
    collider_wall4->SetShapeType(ShapeType::BOX);
    collider_wall4->SetBoxSize({2.0f, 2.0f, 2.0f});
    auto rigidBodyWall4 = wall4->AddComponent<PhysicsBody>();
    rigidBodyWall4->SetStatic(true);

    GameObject *play = currentScene->CreateGameObject("Player NAV TEST");
    play->transform.SetLocalPosition({33.8f, 83.5f, 20.0f});
    auto playMesh = play->AddComponent<MeshInstance>();
    playMesh->LoadModel("res/models/capsule/capsule.obj");
    auto playCollider = play->AddComponent<CollisionShape>();
    playCollider->SetShapeType(ShapeType::CAPSULE);
    playCollider->SetCapsuleParams(1.0f, 4.0f);
    auto playRigidBody = play->AddComponent<PhysicsBody>();
    play->AddComponent<LuaComponent>("res/scripts/player_controller.lua");

    GameObject *player2 = currentScene->CreateGameObject("Agent NAV TEST");
    player2->transform.SetLocalPosition({18.0f, 83.5f, 0.0f});
    auto playerMesh2 = player2->AddComponent<MeshInstance>();
    playerMesh2->LoadModel("res/models/capsule/capsule.obj");
    auto playerCollider2 = player2->AddComponent<CollisionShape>();
    playerCollider2->SetShapeType(ShapeType::CAPSULE);
    playerCollider2->SetCapsuleParams(1.0f, 4.0f);
    auto playerRigidBody2 = player2->AddComponent<PhysicsBody>();
    playerRigidBody2->SetMass(1.0f);

    GameObject *player3 = currentScene->CreateGameObject("Agent 2 NAV TEST");
    player3->transform.SetLocalPosition({-34.0f, 83.5f, 28.0f});
    auto playerMesh3 = player3->AddComponent<MeshInstance>();
    playerMesh3->LoadModel("res/models/capsule/capsule.obj");
    auto playerCollider3 = player3->AddComponent<CollisionShape>();
    playerCollider3->SetShapeType(ShapeType::CAPSULE);
    playerCollider3->SetCapsuleParams(1.0f, 4.0f);
    auto playerRigidBody3 = player3->AddComponent<PhysicsBody>();
    playerRigidBody3->SetMass(1.0f);

    GameObject *player4 = currentScene->CreateGameObject("Agent 3 NAV TEST");
    player4->transform.SetLocalPosition({-34.0f, 83.5f, 20.0f});
    auto playerMesh4 = player4->AddComponent<MeshInstance>();
    playerMesh4->LoadModel("res/models/capsule/capsule.obj");
    auto playerCollider4 = player4->AddComponent<CollisionShape>();
    playerCollider4->SetShapeType(ShapeType::CAPSULE);
    playerCollider4->SetCapsuleParams(1.0f, 4.0f);
    auto playerRigidBody4 = player4->AddComponent<PhysicsBody>();
    playerRigidBody4->SetMass(1.0f);

    plat5->AddComponent<NavigationMesh>();
    plat5->GetComponent<NavigationMesh>()->SetFloor(plat5->GetComponent<MeshInstance>());
    plat5->GetComponent<NavigationMesh>()->AddObstacle(plat6->GetComponent<CollisionShape>());
    plat5->GetComponent<NavigationMesh>()->AddObstacle(plat7->GetComponent<CollisionShape>());
    plat5->GetComponent<NavigationMesh>()->AddObstacle(plat8->GetComponent<CollisionShape>());
    plat5->GetComponent<NavigationMesh>()->Generate();

    play->AddComponent<NavigationTarget>();
    player2->AddComponent<AIAgent>(player2->transform.position);
    player2->GetComponent<AIAgent>()->SetMaxSpeed(4.0f);
    player3->AddComponent<AIAgent>(player3->transform.position);
    player3->GetComponent<AIAgent>()->SetMaxSpeed(2.5f);
    player4->AddComponent<AIAgent>(player4->transform.position);

    GameObject *navSys = currentScene->CreateGameObject("AI System");
    navSys->AddComponent<AISystem>();
    navSys->GetComponent<AISystem>()->SetNavigationMesh(plat5->GetComponent<NavigationMesh>());
    navSys->GetComponent<AISystem>()->SetTarget(play->GetComponent<NavigationTarget>());
    navSys->GetComponent<AISystem>()->RegisterAgent(player2->GetComponent<AIAgent>());
    navSys->GetComponent<AISystem>()->RegisterAgent(player3->GetComponent<AIAgent>());
    navSys->GetComponent<AISystem>()->RegisterAgent(player4->GetComponent<AIAgent>());
}