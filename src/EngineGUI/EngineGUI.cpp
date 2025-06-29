//
// Created by Hubert Klonowski on 15/03/2025.
//

#include "EngineGUI.h"

#include <filesystem>
#include <fstream>
#include <imgui_internal.h>
#include "Components/LuaComponent/LuaComponent.h"
#include "Helpers/Util.h"
#include "Signal/Signals.h"
#include "dBengine/EngineSettings/EngineSettings.h"
#include "dBengine/dBengine.h"

#include <Components/Lights/PointLight/PointLight.h>
#include <Components/Lights/SpotLight/SpotLight.h>
#include <Gizmo/Gizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <imguizmo/ImGuizmo.h>
#include "Components/Lights/LocalLight.h"
#include "Components/TextRenderer/TextRenderer.h"

#include "Components/MeshInstance/MeshInstance.h"
#include <Serializers/SerializeTypes.h>
#include <Serializers/SceneSerializer.h>
#include "Helpers/fonts/IconsFontAwesome4.h"
#include <ResourceManager/ResourceManager.h>
#include <Helpers/TimerHelper/TimerHelper.h>
#include <ImGuiFileDialog.h>
#include <Helpers/Colors/Colors.h>

EngineGUI& EngineGUI::GetInstance() { 
    static EngineGUI instance;
    return instance;
}

void EngineGUI::Init(GLFWwindow* window, const char* glslVersion) {
    TimerHelper initTimer("EngineGUI::Init");

    this->uuid = UUID::generateUUID();
    this->window = window;
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    Signals::Editor_ExitFullscreen.connect(this->uuid,[this] { editorFullscreen = false; });

    Signals::Editor_SetTheme.connect(this->uuid,[this](int i) { theme.SetTheme(i); });

    Signals::Editor_SetScriptPathToEdit.connect(this->uuid,[this](std::string path) {
        scriptPath = path;
        loadScript();
        codeEditorEnabled = true;
    });

    Signals::Engine_CheckFramebufferDimensions.connect(this->uuid,[this]() { checkFramebufferDimensions(); });

    ImGuiIO& io = ImGui::GetIO();
    // Enable Multi-Viewport / Platform Windows
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    io.Fonts->Clear();

    float baseFontSize = 15.0f; 
    float iconFontSize =
            (baseFontSize) * 3.0f /
            4.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly


    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_16_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.GlyphMinAdvanceX = iconFontSize;

    ff_Inter = FontFamily("Inter");
    ff_FiraCode = FontFamily("FiraCode");
    ff_Roboto = FontFamily("Roboto");

    ff_Inter.AddFont(REGULAR, io.Fonts->AddFontFromFileTTF("res/fonts/Inter/Inter-Regular.ttf", baseFontSize));
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FA, iconFontSize, &icons_config, icons_ranges);

    ff_FiraCode.AddFont(REGULAR, io.Fonts->AddFontFromFileTTF("res/fonts/FiraCode/FiraCode-Regular.ttf", baseFontSize));

    ff_FiraCode.AddFont(LIGHT, io.Fonts->AddFontFromFileTTF("res/fonts/FiraCode/FiraCode-Light.ttf", baseFontSize));
    ff_FiraCode.AddFont(BOLD, io.Fonts->AddFontFromFileTTF("res/fonts/FiraCode/FiraCode-Bold.ttf", baseFontSize));

    ff_Inter.AddFont(ITALIC, io.Fonts->AddFontFromFileTTF("res/fonts/Inter/Inter-Italic.ttf", baseFontSize));
    ff_Inter.AddFont(LIGHT, io.Fonts->AddFontFromFileTTF("res/fonts/Inter/Inter-Light.ttf", baseFontSize));
    ff_Inter.AddFont(THIN, io.Fonts->AddFontFromFileTTF("res/fonts/Inter/Inter-Thin.ttf", baseFontSize));
    ff_Inter.AddFont(BOLD, io.Fonts->AddFontFromFileTTF("res/fonts/Inter/Inter-Bold.ttf", baseFontSize));

    //io.FontDefault = ff_Inter.GetFont(REGULAR);


    // Setup style
    ImGui::StyleColorsDark();
    theme.SetTheme(EngineSettings::GetEngineSettings().selectedTheme);
    ImGuiStyle &style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplOpenGL3_CreateFontsTexture();

    logo = ResourceManager::GetInstance().LoadTextureFromFile("res/textures/dBengine_logo_white.png");

    //set styling for file dialog
    ImGuiFileDialog* fileDialog = ImGuiFileDialog::Instance();

    fileDialog->SetFileStyle(IGFD_FileStyleByTypeDir, nullptr, Colors::White, ICON_FA_FOLDER);

    ImVec4 fileColor = Colors::White;

    // folders
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".png", fileColor, ICON_FA_FILE_IMAGE_O);
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".jpg", fileColor, ICON_FA_FILE_IMAGE_O);
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".jpeg", fileColor, ICON_FA_FILE_IMAGE_O);
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".gif", fileColor, ICON_FA_FILE_IMAGE_O);
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".hdr", fileColor, ICON_FA_FILE_IMAGE_O);

    // models
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".obj", fileColor, ICON_FA_CUBE);
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".fbx", fileColor, ICON_FA_CUBE);
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".dae", fileColor, ICON_FA_CUBE);
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".gltf", fileColor, ICON_FA_CUBE);

    // code
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".cpp", fileColor, ICON_FA_FILE_CODE_O);
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".h", fileColor, ICON_FA_FILE_CODE_O);
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".lua", fileColor, ICON_FA_FILE_CODE_O);

    // audio
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".mp3", fileColor, ICON_FA_FILE_AUDIO_O);
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".wav", fileColor, ICON_FA_FILE_AUDIO_O);
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".ogg", fileColor, ICON_FA_FILE_AUDIO_O);

    // scene
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".scene", fileColor, ICON_FA_CUBES);

    // resource
    fileDialog->SetFileStyle(IGFD_FileStyleByExtention, ".res", fileColor, ICON_FA_DATABASE);



}

void EngineGUI::Update(float deltaTime) {
    float fps = ImGui::GetIO().Framerate;
    frameTimes.push_back(deltaTime);
    if (frameTimes.size() > 100) {
        frameTimes.erase(frameTimes.begin());
    }

    // listen for shortcut input to save the script
    if (codeEditorEnabled && SAVE_SHORTCUT && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        saveScript();
    }

    if (codeEditorEnabled && SAVE_SHORTCUT && ImGui::IsKeyPressed(ImGuiKey_R, false)) {
        if (scriptPath != "") {
            saveScript();
            reloadCode(std::filesystem::path(scriptPath).filename().extension().string());
        }
    }

}

void EngineGUI::Render() {
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
        return; // Skip rendering entirely
    }
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    ImGuiViewport *viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::Begin("DockSpace", NULL,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoResize);
    
   
    renderDockSpace();

    //drawviewport
    m_ViewportPanel.Draw();
    m_EditorMenuPanel.Draw();
    m_SettingsPanel.Draw();
    m_AnimationLibraryPanel.Draw();
    m_AnimationTransitionPanel.Draw();
    m_InputManagerPanel.Draw();
    m_ThemeEditorPanel.Draw();
    m_PreviewPanel.Draw();

    if (!editorFullscreen) {

        m_ScenePanel.Draw();

        fileExplorer.Draw();

        m_InspectorPanel.Draw();

        if (showingDebugWindow) {
            m_DebugPanel.Draw();

            m_ConsolePanel.Draw();
        }
        if (codeEditorEnabled) {
            m_CodeEditorPanel.Draw();
        }
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // Obsługa dodatkowych viewportów

    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}


void EngineGUI::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void EngineGUI::reloadCode(const std::string &extension) { 
    if (extension == ".lua") {
        Signals::ReloadScript.emit(scriptPath);
        return;
    }

    if (extension == ".vert" || extension == ".frag" || extension == ".vs" || extension == ".fs" || extension == ".geom") {
        Signals::ReloadShaders.emit();
        return;
    }
}

void EngineGUI::SetScene(Scene* scene) {
    this->scene = scene; 
    m_ViewportPanel.SetScene(scene);
}

void EngineGUI::SetCamera(Camera *camera) { 
    this->camera = camera; 
    m_ViewportPanel.SetCamera(camera);
}



void EngineGUI::loadScript() {
    std::ifstream file(scriptPath);
    if (file) {
        file.read(scriptBuffer, BUF_SIZE - 1);
        scriptBuffer[file.gcount()] = '\0';
        originalScript = scriptBuffer;
        isModified = false;
    } else {
        std::cerr << "Failed to open script: " << scriptPath << std::endl;
    }
    file.close();
}

void EngineGUI::saveScript() {
    std::ofstream file(scriptPath);
    if (file) {
        file.write(scriptBuffer, strlen(scriptBuffer));
        originalScript = scriptBuffer;
        isModified = false;
        std::cout << "Script "<< scriptPath << " saved!" << std::endl;
    } else {
        std::cerr << "Failed to open script: " << scriptPath << std::endl;
    }
    file.close();
}


void EngineGUI::renderDockSpace() {
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0, 0));
    static auto firstTime = true;
    if (firstTime) {
        firstTime = false;

        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

        auto dock_id_left = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.15f, nullptr, &dockspace_id);
        auto dock_id_right = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.30f, nullptr, &dockspace_id);
        auto dock_id_bottom = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Down, 0.1f, nullptr, &dockspace_id);
        
        auto dock_id_left_bottom =
                ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Down, 0.4f, nullptr, &dock_id_left);

        auto dock_id_left_top =
                ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Up, 0.19f, nullptr, &dock_id_left);
        ImGuiDockNode *logoNode = ImGui::DockBuilderGetNode(dock_id_left_top);
        logoNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar | ImGuiDockNodeFlags_NoResize;

        ImGuiDockNode *leftNode = ImGui::DockBuilderGetNode(dock_id_left);
        leftNode->LocalFlags |=  ImGuiDockNodeFlags_NoResize;

        ImGui::DockBuilderDockWindow("Logo", dock_id_left_top);
        ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_id_left);
        ImGui::DockBuilderDockWindow("Debug", dock_id_right);
        ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
        ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
        ImGui::DockBuilderDockWindow("Viewport", dockspace_id);
        ImGui::DockBuilderDockWindow("Code Editor", dockspace_id);
        ImGui::DockBuilderDockWindow("File Explorer", dock_id_left_bottom);


        ImGui::DockBuilderFinish(dockspace_id);
    }
}

void EngineGUI::checkFramebufferDimensions() {
    const float window_width = ImGui::GetContentRegionAvail().x;
    const float window_height = ImGui::GetContentRegionAvail().y;

    // Resize the framebuffer to match the ImGui window size
    // //TODO: OPTIMIZE THIS, CURRENTLY ITS CHECKING EVERY FRAME FOR CHANGES
    if (framebuffer_height != window_height || framebuffer_width != window_width) {
        framebuffer_height = window_height;
        framebuffer_width = window_width;
        glViewport(0, 0, window_width, window_height);
        if (window_width == 0 || window_height == 0) {
            return; // Skip when minimized
        }
        Ref::AspectRatio = static_cast<float>((float) window_width / (float) window_height);
        Signals::Camera_UpdateAspectRatio.emit(Ref::AspectRatio);
        auto& renderer = dBrender::GetInstance();
        renderer.framebuffer.RescaleFrameBuffer(window_width, window_height);
        renderer.renderFrameBuffer.RescaleFrameBuffer(window_width, window_height);
        renderer.framebufferMultisample.RescaleFrameBuffer(window_width, window_height);
        renderer.bloomRenderer.Rescale(window_width, window_height);
        renderer.HUDBloomRenderer.Rescale(window_width, window_height);
        renderer.UIFramebuffer.RescaleFrameBuffer(window_width, window_height);
    }
}


// Ray-triangle intersection function (MöllerMilch–Trumbore)
bool RayTriangleIntersect(const glm::vec3 &orig, const glm::vec3 &dir, const glm::vec3 &v0, const glm::vec3 &v1,
                          const glm::vec3 &v2, float &t) {
    const float EPSILON = 0.000001f;
    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(dir, edge2);
    float a = glm::dot(edge1, h);
    if (a > -EPSILON && a < EPSILON)
        return false; // Ray is parallel to triangle
    float f = 1.0f / a;
    glm::vec3 s = orig - v0;
    float u = f * glm::dot(s, h);
    if (u < 0.0f || u > 1.0f)
        return false;
    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(dir, q);
    if (v < 0.0f || u + v > 1.0f)
        return false;
    t = f * glm::dot(edge2, q);
    return t > EPSILON;
}

GameObject *EngineGUI::PickObject(const std::vector<GameObject*> &gameObjects, float mouseX,
                                  float mouseY, float windowWidth, float windowHeight) {
    GameObject *result = nullptr;
    float closestDistance = std::numeric_limits<float>::max(); // [for closest object]

    // Step 1: Set up the ray in world space
    glm::mat4 cameraView = camera->GetViewMatrix();
    glm::mat4 cameraProj = camera->GetProjectionMatrix();

    // mouse to NDC (Normalized Device Coordinates)
    float x = (2.0f * mouseX) / windowWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / windowHeight;
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    // eye space
    glm::mat4 invProj = glm::inverse(cameraProj);
    glm::vec4 rayEye = invProj * rayClip;
    rayEye.z = -1.0f; // "Forward"
    rayEye.w = 0.0f; // Dir 

    // world space
    glm::mat4 invView = glm::inverse(cameraView);
    glm::vec4 rayWorld = invView * rayEye;
    glm::vec3 rayDirection = glm::normalize(glm::vec3(rayWorld));
    glm::vec3 rayOrigin = glm::vec3(invView * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    
    // Step 2: Check intersection with each GameObject's meshes

    for (int i = 0; i < gameObjects.size(); i++) {
        GameObject *gameObject = gameObjects[i];

        if (!gameObject || !gameObject->m_enabled || (gameObject->GetComponent<MeshInstance>() && !gameObject->GetComponent<MeshInstance>()->enabled))
            continue;

        Transform transform = gameObject->transform;

        MeshInstance *meshInstance = gameObject->GetComponent<MeshInstance>();
        if (!meshInstance || !meshInstance->model || meshInstance->model->Meshes.empty())
            continue;

        // inverse -> world to local
        glm::mat4 M_inv = glm::inverse(transform.modelMatrix);

        // ray origin/dir to local space too by multi. inv matrix
        glm::vec3 O_l = glm::vec3(M_inv * glm::vec4(rayOrigin, 1.0f));
        glm::vec3 D_l = glm::vec3(M_inv * glm::vec4(rayDirection, 0.0f));

        // check every mesh
        for (auto &mesh: meshInstance->model->Meshes) {
            // check every triangle in mesh
            for (size_t i = 0; i < mesh.indices.size(); i += 3) {
                glm::vec3 v0 = mesh.vertices[mesh.indices[i]].position;
                glm::vec3 v1 = mesh.vertices[mesh.indices[i + 1]].position;
                glm::vec3 v2 = mesh.vertices[mesh.indices[i + 2]].position;

                float t;
                if (RayTriangleIntersect(O_l, D_l, v0, v1, v2, t)) {
                    if (t > 0 && t < closestDistance) {
                        closestDistance = t;
                        result = gameObject;
                    }
                }
            }
        }
    }
    if (result) {
        GameObject *tempObject = result;
        while (tempObject->parent) {
            if (tempObject->pathToScene != "") {
                result = tempObject;
                break;
            }
            tempObject = tempObject->parent;
        }
    }
    return result;
}



SettingsPanel &EngineGUI::GetSettingsPanel() { return m_SettingsPanel; }

EditorMenuPanel &EngineGUI::GetEditorMenuPanel() { return m_EditorMenuPanel; }

AnimationLibraryPanel &EngineGUI::GetAnimationLibraryPanel() { return m_AnimationLibraryPanel; }

AnimationTransitionPanel &EngineGUI::GetAnimationTransitionPanel() { return m_AnimationTransitionPanel; }

InputManagerPanel &EngineGUI::GetInputManagerPanel() { return m_InputManagerPanel; }

ThemeEditorPanel &EngineGUI::GetThemeEditorPanel() { return m_ThemeEditorPanel; }

PreviewPanel &EngineGUI::GetPreviewPanel() { return m_PreviewPanel; }

FontFamily &EngineGUI::GetCodeFontFamily() { return ff_FiraCode; }

FontFamily &EngineGUI::GetInterfaceFontFamily() { return ff_Inter; }

Scene *EngineGUI::GetScene() { return scene; }

