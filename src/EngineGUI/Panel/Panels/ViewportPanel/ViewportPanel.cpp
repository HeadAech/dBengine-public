#include "ViewportPanel.h"
#include <dBengine/EngineSettings/EngineSettings.h>
#include <imgui_internal.h>
#include <Helpers/fonts/IconsFontAwesome4.h>
#include <Gizmo/Gizmo.h>
#include <InputManager/Input.h>
#include <dBrender/dBrender.h>
#include <Signal/Signals.h>
#include <EngineGUI/EngineGUI.h>
#include "Scene/Scene.h"
#include <Singletons/Ref/Ref.h>
#include <Helpers/Colors/Colors.h>
#include <ImGuiFileDialog.h>

ViewportPanel::ViewportPanel() { SetName("Viewport"); }

void ViewportPanel::Draw() {
    EngineSettings &engineSettings = EngineSettings::GetEngineSettings();
    EngineGUI &gui = EngineGUI::GetInstance();

    const std::vector<GameObject *> &gameObjects = dBrender::GetInstance().m_flatGameObjects; 

    ImGui::Begin(GetName().c_str());
    {


        if (Panel::ColoredButton(ICON_FA_PLAY " Play", Colors::DarkGreen))
        {
            std::string_view currentScenePath = gui.GetScene()->Path; 
            if (currentScenePath.empty()) {
                EngineDebug::GetInstance().PrintError("There is no such path set! Please save the scene first.");

                if (true) {
                    gui.GetEditorMenuPanel().SetSaveModalVisibility(true);
                    gui.selectedGameObject = nullptr;
                }
            } else {
                EngineDebug::GetInstance().PrintInfo(currentScenePath.data());
                std::string cmd = "dBengine.exe --no-editor --scene \"" + std::string(currentScenePath) + "\"";
                std::system(cmd.c_str());
            }
        }
        
        gui.GetEditorMenuPanel().DrawSaveModal();
        
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::Text("Gizmos: ");
        ImGui::SameLine();
        ImGui::BeginGroup();

        ImVec4 toggledColor = {0.5, 0.5, 0.7f, 1.0f};

 
        bool tempViewCubeFlag = m_viewCubeCameraFlag;
        if (tempViewCubeFlag) {
            ImGui::PushStyleColor(ImGuiCol_Button, toggledColor); // Green
        }

        if (ImGui::Button(ICON_FA_CUBE)) {
            m_viewCubeCameraFlag = !m_viewCubeCameraFlag;
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Render ViewCube");
        }
        if (tempViewCubeFlag) {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();
        bool useSnap = Gizmo::GetSnapToggled();

        if (useSnap) {
            ImGui::PushStyleColor(ImGuiCol_Button, toggledColor); // Green
        }


        if (ImGui::Button(ICON_FA_MAGNET)) {
            if (useSnap) {
                Gizmo::SetSnapToggle(false);
            } else {
                Gizmo::SetSnapToggle(true);
            }
        }

        if (useSnap) {
            ImGui::PopStyleColor();
        }


        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("TOGGLE snapping in gizmo - [LEFT SHIFT]");
        }

        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        ImGuizmo::OPERATION gizmoOperation = Gizmo::GetCurrentOperation();
        ImGuizmo::MODE gizmoMode = Gizmo::GetCurrentMode();
        ImGui::SameLine();

        if (gizmoOperation == ImGuizmo::TRANSLATE) {
            ImGui::PushStyleColor(ImGuiCol_Button, toggledColor); // Green
        }

        if (ImGui::Button(ICON_FA_ARROWS)) {
            Gizmo::SetGizmoOperation(ImGuizmo::TRANSLATE);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Change gizmo operation to TRANSLATE - [E]");
        }

        if (gizmoOperation == ImGuizmo::TRANSLATE) {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        if (gizmoOperation == ImGuizmo::ROTATE) {
            ImGui::PushStyleColor(ImGuiCol_Button, toggledColor); // Green
        }

        if (ImGui::Button(ICON_FA_UNDO)) {
            Gizmo::SetGizmoOperation(ImGuizmo::ROTATE);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Change gizmo operation to ROTATE - [R]");
        }

        if (gizmoOperation == ImGuizmo::ROTATE) {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        if (gizmoOperation == ImGuizmo::SCALE) {
            ImGui::PushStyleColor(ImGuiCol_Button, toggledColor); // Green
        }

        if (ImGui::Button(ICON_FA_EXPAND)) {
            Gizmo::SetGizmoOperation(ImGuizmo::SCALE);
        }

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Change gizmo operation to SCALE - [S]");
        }

        if (gizmoOperation == ImGuizmo::SCALE) {
            ImGui::PopStyleColor();
        }

        ImGui::SameLine();

        if (gizmoOperation == ImGuizmo::TRANSLATE || gizmoOperation == ImGuizmo::ROTATE) {

            if (gizmoMode == ImGuizmo::LOCAL) {
                ImGui::PushStyleColor(ImGuiCol_Button, toggledColor); // Green
            }
            if (ImGui::Button(ICON_FA_HOME)) {
                Gizmo::SetGizmoMode(ImGuizmo::LOCAL);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Change gizmo mode to LOCAL");
            }
            if (gizmoMode == ImGuizmo::LOCAL) {
                ImGui::PopStyleColor();
            }
            ImGui::SameLine();
            if (gizmoMode == ImGuizmo::WORLD) {
                ImGui::PushStyleColor(ImGuiCol_Button, toggledColor); // Green
            }
            if (ImGui::Button(ICON_FA_GLOBE)) {
                Gizmo::SetGizmoMode(ImGuizmo::WORLD);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Change gizmo mode to WORLD");
            }
            if (gizmoMode == ImGuizmo::WORLD) {
                ImGui::PopStyleColor();
            }
        }
            
        ImGui::EndGroup();

        ImGui::BeginChild("OpenGL Frame");

        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup)) {
            //if (Input::GetInstance().IsActionJustPressed("mouse2")) {
            //    m_movingCameraInEditor = true;
            //    Input::GetInstance().SetCursorLocked(true);
            //}

            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 windowPos = ImGui::GetWindowPos();
            float mouseX = mousePos.x - windowPos.x;
            float mouseY = mousePos.y - windowPos.y;

            // Check for hover and click
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                if (!gui.selectedGameObject || !ImGuizmo::IsOver()) {
                    gui.selectedGameObject =
                            gui.PickObject(gameObjects, mouseX, mouseY, ImGui::GetContentRegionAvail().x,
                                            ImGui::GetContentRegionAvail().y);
                }
            }
        }

        //if (m_movingCameraInEditor && Input::GetInstance().IsActionJustReleased("mouse2")) {
        //    m_movingCameraInEditor = false;
        //    Input::GetInstance().SetCursorLocked(false);
        //}

        const float window_width = ImGui::GetContentRegionAvail().x;
        const float window_height = ImGui::GetContentRegionAvail().y;

        Signals::Engine_CheckFramebufferDimensions.emit();

        ImVec2 imagePos = ImGui::GetCursorScreenPos();

        ImGui::Image((ImTextureID) dBrender::GetInstance().renderFrameBuffer.getFrameTexture(),
                        ImVec2(window_width, window_height), ImVec2(0, 1), ImVec2(1, 0));

        ImVec2 mouseScreen = ImGui::GetMousePos();

        ImVec2 mouseInViewport = {mouseScreen.x - imagePos.x, mouseScreen.y - imagePos.y};

        Ref::MousePosition = {mouseInViewport.x, mouseInViewport.y};

        // Use Gizmo methods for transformation handling
        glm::mat4 identMat = glm::mat4(1.0f);
        glm::mat4 cameraView = m_camera->GetViewMatrix();
        glm::mat4 cameraProjection = m_camera->GetProjectionMatrix();

        Gizmo::TransformStart(cameraView, cameraProjection, identMat);


        if (gui.selectedGameObject && gui.selectedGameObject->m_enabled &&
            (!gui.selectedGameObject->GetComponent<MeshInstance>() ||
                (gui.selectedGameObject->GetComponent<MeshInstance>() &&
                (gui.selectedGameObject->GetComponent<MeshInstance>()->enabled))) &&
            (!gui.selectedGameObject->GetComponent<Camera>() ||
                (gui.selectedGameObject->GetComponent<Camera>() &&
                !gui.selectedGameObject->GetComponent<Camera>()->isUsed))) {

            try {
                std::string uuid = gui.selectedGameObject->GetUUID(); 

                ImGuizmo::PushID(uuid.c_str());
                Gizmo::EditTransform(cameraView, cameraProjection, gui.selectedGameObject);
                ImGuizmo::PopID();
            } catch (...) {
                gui.selectedGameObject = nullptr;
            }
        }

        Gizmo::TransformEnd();
        if (m_viewCubeCameraFlag) {
            Gizmo::DrawViewManipulate(m_camera, 1.0);
        }

        ImGui::EndChild();
    }
    ImGui::End();
    }


void ViewportPanel::SetCamera(Camera *camera) { this->m_camera = camera; }

void ViewportPanel::SetScene(Scene *scene) { m_Scene = scene; }
