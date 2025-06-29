//
// Created by Hubert Klonowski on 15/03/2025.
//

#ifndef ENGINEGUI_H
#define ENGINEGUI_H

#include "imgui.h"
#include "dBrender/dBrender.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"
#include "Scene/Scene.h"
#include "EngineTheme.h"
#include "FileExplorer/FileExplorer.h"

#include "Font/Font.h"

//panels
#include "Panel/Panels/ViewportPanel/ViewportPanel.h"
#include "Panel/Panels/InspectorPanel/InspectorPanel.h"
#include "Panel/Panels/ConsolePanel/ConsolePanel.h"
#include "Panel/Panels/DebugPanel/DebugPanel.h"
#include "Panel/Panels/ScenePanel/ScenePanel.h"
#include "Panel/Panels/SettingsPanel/SettingsPanel.h"
#include "Panel/Panels/CodeEditorPanel/CodeEditorPanel.h"
#include "Panel/Panels/EditorMenuPanel/EditorMenuPanel.h"
#include "Panel/Panels/AnimationLibraryPanel/AnimationLibraryPanel.h"
#include "Panel/Panels/AnimationTransitionPanel/AnimationTransitionPanel.h"
#include "Panel/Panels/InputManagerPanel/InputManagerPanel.h"
#include "Panel/Panels/ThemeEditorPanel/ThemeEditorPanel.h"
#include "Panel/Panels/PreviewPanel/PreviewPanel.h"

#ifdef __APPLE__
#define SAVE_SHORTCUT ImGui::GetIO().KeySuper  // Cmd key on macOS
#else
#define SAVE_SHORTCUT ImGui::GetIO().KeyCtrl   // Ctrl key on Windows/Linux
#endif

/// <summary>
/// Class managing engine's editor's GUI.
/// </summary>
class EngineGUI {
    std::string uuid;
    Scene* scene;
    Camera *camera;
    GLFWwindow* window;

    bool showingDebugWindow = true;

    // dock space
    void renderDockSpace();
    float framebuffer_width = 0;
    float framebuffer_height = 0;
    void checkFramebufferDimensions();


    bool movingCameraInEditor = false;

    FileExplorer fileExplorer;


    // panels
    ViewportPanel m_ViewportPanel;
    InspectorPanel m_InspectorPanel;
    ConsolePanel m_ConsolePanel;
    DebugPanel m_DebugPanel;
    ScenePanel m_ScenePanel;
    SettingsPanel m_SettingsPanel;
    CodeEditorPanel m_CodeEditorPanel;
    EditorMenuPanel m_EditorMenuPanel;
    AnimationLibraryPanel m_AnimationLibraryPanel;
    AnimationTransitionPanel m_AnimationTransitionPanel;
    InputManagerPanel m_InputManagerPanel;
    ThemeEditorPanel m_ThemeEditorPanel;
    PreviewPanel m_PreviewPanel;

    //fonts
    FontFamily ff_Inter;
    FontFamily ff_FiraCode;
    FontFamily ff_Roboto;

public:
    /// <summary>
    /// Logo for the engine.
    /// </summary>
    std::shared_ptr<Texture> logo;

    /// <summary>
    /// Current theme of the engine.
    /// </summary>
    EngineTheme theme;

    /// <summary>
    /// Debug list of last frame times.
    /// </summary>
    std::vector<float> frameTimes;
    
    //editor
    bool editorFullscreen = false;
    GameObject *selectedGameObject = nullptr;
    bool changedSelectedGameObject = false;
    GameObject *PickObject(const std::vector<GameObject*> &gameObjects, float mouseX,
                           float mouseY, float windowWidth, float windowHeight);

    /// <summary>
    /// Is code editor enabled.
    /// </summary>
    bool codeEditorEnabled = false;
    std::string scriptPath;
    constexpr size_t static BUF_SIZE = 50000;
    char scriptBuffer[BUF_SIZE] = {};
    std::string originalScript;
    bool isModified = false;

    /// <summary>
    /// Load script for use in the code editor.
    /// </summary>
    void loadScript();

    /// <summary>
    /// Save the script to the file system.
    /// </summary>
    void saveScript();

    /// <summary>
    /// Reload current script in the code editor.
    /// <para>
    ///     If the script is modified, it will prompt to save changes.
    /// </para>
    /// </summary>
    /// <param name="extension"></param>
    void reloadCode(const std::string &extension);

    EngineGUI() = default;

    /// <summary>
    /// Initializes the EngineGUI.
    /// </summary>
    /// <param name="window">Pointer to GLFW window</param>
    /// <param name="glslVersion">Version of GLSL</param>
    void Init(GLFWwindow* window, const char* glslVersion);

    /// <summary>
    /// Updates the EngineGUI.
    /// </summary>
    /// <param name="deltaTime">Time since last frame</param>
    void Update(float deltaTime);

    /// <summary>
    /// Renders the EngineGUI.
    /// </summary>
    void Render();

    /// <summary>
    /// Cleans up the EngineGUI resources on shutdown.
    /// </summary>
    void Shutdown();

    /// <summary>
    /// Sets the current scene for the EngineGUI.
    /// </summary>
    /// <param name="scene">Pointer to scene</param>
    void SetScene(Scene* scene);

    /// <summary>
    /// Sets the camera for the EngineGUI.
    /// </summary>
    /// <param name="camera">Pointer to camera</param>
    void SetCamera(Camera *camera);

    /// <summary>
    /// Returns the singleton instance of EngineGUI.
    /// </summary>
    /// <returns>Singleton (EngineGUI)</returns>
    static EngineGUI &GetInstance();
    EngineGUI(const EngineGUI &) = delete;
    EngineGUI &operator=(const EngineGUI &) = delete;

    /// <summary>
    /// Returns the pointer to the current scene.
    /// </summary>
    /// <returns>Pointer (Scene)</returns>
    Scene *GetScene();
    std::vector<Scene*> GetScenes();

    // panel getters
    
    /// <summary>
    /// Returns the reference to Settings Panel.
    /// </summary>
    /// <returns>SettingsPanel</returns>
    SettingsPanel &GetSettingsPanel();

    /// <summary>
    /// Returns the reference to EditorMenuPanel.
    /// </summary>
    /// <returns>EditorMenuPanel</returns>
    EditorMenuPanel &GetEditorMenuPanel();

    /// <summary>
    /// Returns the reference to the Animation Library Panel.
    /// </summary>
    /// <returns>AnimationLibraryPanel</returns>
    AnimationLibraryPanel &GetAnimationLibraryPanel();

    /// <summary>
    /// Returns the reference to the Animation Transition Panel.
    /// </summary>
    /// <returns>AnimationTransitionPanel</returns>
    AnimationTransitionPanel &GetAnimationTransitionPanel();

    /// <summary>
    /// Returns the reference to the Input Manager Panel.
    /// </summary>
    /// <returns>Input Manager Panel</returns>
    InputManagerPanel &GetInputManagerPanel();

    /// <summary>
    /// Returns the reference to the Theme Editor Panel.
    /// </summary>
    /// <returns>Theme Editor Panel</returns>
    ThemeEditorPanel &GetThemeEditorPanel();

    /// <summary>
    /// Returns the reference to the Preview Panel.
    /// </summary>
    /// <returns>PreviewPanel</returns>
    PreviewPanel &GetPreviewPanel();

    /// <summary>
    /// Returns the reference to the Code Font Family.
    /// </summary>
    /// <returns>FontFamily</returns>
    FontFamily &GetCodeFontFamily();

    /// <summary>
    /// Returns the reference to the Interface Font Family.
    /// </summary>
    /// <returns>FontFamily</returns>
    FontFamily &GetInterfaceFontFamily();
};



#endif //ENGINEGUI_H
