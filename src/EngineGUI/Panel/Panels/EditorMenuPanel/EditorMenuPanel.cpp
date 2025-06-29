#include "EditorMenuPanel.h"
#include <imgui_internal.h>
#include <Helpers/fonts/IconsFontAwesome4.h>
#include <Signal/Signals.h>
#include <EngineGUI/EngineGUI.h>
#include <dBengine/EngineSettings/EngineSettings.h>
#include <Helpers/Colors/Colors.h>
#include "dBengine/EngineDebug/EngineDebug.h"
#include <ImGuiFileDialog.h>

EditorMenuPanel::EditorMenuPanel() { SetName("Editor Menu"); }

void EditorMenuPanel::Draw() {

    EngineGUI &gui = EngineGUI::GetInstance();
    ImGui::SetNextWindowSizeConstraints(ImVec2(100, 120), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowDockID(ImGui::GetID("LeftDockSpace"), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Logo", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {

        ImGui::Image(gui.logo->id, ImVec2(140, 60));

        ImGui::SameLine();

        ImGui::BeginGroup();

        if (ImGui::BeginMenu("File")) {

            if (ImGui::MenuItem(ICON_FA_FILE_O " New Scene")) {
                Signals::NewScene.emit("New Scene");
            }

            if (ImGui::MenuItem(ICON_FA_FLOPPY_O " Save Scene")) {
                m_SaveModalVisible = true;
                gui.selectedGameObject = nullptr;
            }
            if (ImGui::MenuItem(ICON_FA_FILE " Load Scene")) {
                m_LoadModalVisible = true;
                gui.selectedGameObject = nullptr;
            }

            ImGui::EndMenu();
        }

        DrawSaveModal();
        DrawLoadModal();

        if (ImGui::BeginMenu("Edit")) {

            if (ImGui::MenuItem(ICON_FA_COG " Engine Settings")) {
                gui.GetSettingsPanel().Open();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            bool inFullscreen = EngineSettings::GetEngineSettings().windowInFullscreen;

            if (ImGui::MenuItem(ICON_FA_EXPAND " Fullscreen", nullptr, &inFullscreen)) {
                Signals::Engine_SetWindowFullscreen.emit(inFullscreen);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("ALT + ENTER to toggle fullscreen");
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Tools")) {
            
            if (ImGui::MenuItem(ICON_FA_FILM " Animation Library", nullptr)) {
                gui.GetAnimationLibraryPanel().Open();
            }

            if (ImGui::MenuItem(ICON_FA_KEYBOARD_O " Input Manager", nullptr)) {
                gui.GetInputManagerPanel().Open();
            }

            ImGui::SeparatorText("Other");

            if (ImGui::MenuItem(ICON_FA_PAINT_BRUSH " Theme Editor", nullptr)) {
                gui.GetThemeEditorPanel().Open();
            }


            ImGui::EndMenu();
        }

        ImGui::EndGroup();

        

        
        ImGui::End();
    }

}

void EditorMenuPanel::DrawSaveModal() {
    if (m_SaveModalVisible) {
        ImGui::OpenPopup("Save Scene");
        m_SaveModalVisible = false;
    }

    //always center window
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
                            ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Save Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        
        ImGui::Text("Path: ");

        ImGui::PushItemWidth(-1);
        ImGui::BeginGroup();
        if (ImGui::Button(ICON_FA_FOLDER ""))
        {
            IGFD::FileDialogConfig config;
            config.path = "./res/scenes";
            ImGuiFileDialog::Instance()->OpenDialog("SaveScene", "Save Scene...", ".scene,.yaml", config);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Choose a path to save the scene...");
        }
        ImGui::SameLine();
        ImGui::Text("%s", m_PathOfSavedScene.empty() ? "No scene selected" : m_PathOfSavedScene.c_str());
        
        if (Util::fileExists(m_PathOfSavedScene)) {
            Panel::ColoredText("File will be overwritten!", Colors::DarkRed);
        }
        ImGui::EndGroup();
        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float windowWidth = ImGui::GetWindowSize().x;
        float buttonWidth = 150.0f; // Or calculate via ImGui::CalcTextSize + padding
        float buttonX = (windowWidth - buttonWidth) * 0.5f;

        ImGui::SetCursorPosX(buttonX);

        ImGui::BeginGroup();

        ImGui::BeginDisabled(m_PathOfSavedScene.empty());
        if (Panel::ColoredButton("Save", Colors::DarkGreen, ImVec2(buttonWidth, 0))) {
            Signals::SceneToFile.emit(m_PathOfSavedScene);
            m_PathOfSavedScene = "";
            ImGuiFileDialog::Instance()->Close();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        if (Panel::ColoredButton("Cancel", Colors::DarkRed, ImVec2(buttonWidth, 0))) {
            m_PathOfSavedScene = "";
            ImGuiFileDialog::Instance()->Close();
            ImGui::CloseCurrentPopup();
        }


        ImGui::EndGroup();

        // save scene dialog
        if (ImGuiFileDialog::Instance()->Display("SaveScene", NULL, ImVec2(600, 300)))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                m_PathOfSavedScene = Util::getRelativePath(ImGuiFileDialog::Instance()->GetFilePathName()).string();
            }
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::EndPopup();
    }
    
}

void EditorMenuPanel::DrawLoadModal() {
    if (m_LoadModalVisible) {
        ImGui::OpenPopup("Load Scene");
        m_LoadModalVisible = false;
    }
    //always center window
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Load Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Path: ");

        ImGui::PushItemWidth(-1);
        ImGui::BeginGroup();
        if (ImGui::Button(ICON_FA_FOLDER ""))
        {
            IGFD::FileDialogConfig config;
            config.path = "./res/scenes";
            ImGuiFileDialog::Instance()->OpenDialog("LoadScene", "Load scene...", "Scene files (*.scene *.yaml){.scene,.yaml}", config);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Choose a path to load the scene...");
        }
        ImGui::SameLine();
        ImGui::Text("%s", m_PathOfLoadedScene.empty() ? "No scene selected" : m_PathOfLoadedScene.c_str());
        
        ImGui::EndGroup();
        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float windowWidth = ImGui::GetWindowSize().x;
        float buttonWidth = 150.0f; // Or calculate via ImGui::CalcTextSize + padding
        float buttonX = (windowWidth - buttonWidth) * 0.5f;

        ImGui::SetCursorPosX(buttonX);

        ImGui::BeginGroup();

        ImGui::BeginDisabled(m_PathOfLoadedScene.empty());
        if (Panel::ColoredButton("Load", Colors::DarkGreen, ImVec2(buttonWidth, 0))) {
            
            Signals::FileToScene.emit(m_PathOfLoadedScene);
            //Signals::FileToScene.emit(m_PathOfLoadedScene);
            m_PathOfLoadedScene = "";
            ImGuiFileDialog::Instance()->Close();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        if (Panel::ColoredButton("Cancel", Colors::DarkRed, ImVec2(buttonWidth, 0))) {
            m_PathOfLoadedScene = "";
            ImGuiFileDialog::Instance()->Close();
            ImGui::CloseCurrentPopup();
        }


        ImGui::EndGroup();

        // load scene dialog
        if (ImGuiFileDialog::Instance()->Display("LoadScene", NULL, ImVec2(600, 300)))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                m_PathOfLoadedScene = Util::getRelativePath(ImGuiFileDialog::Instance()->GetFilePathName()).string();
            }
            ImGuiFileDialog::Instance()->Close();
        }


        ImGui::EndPopup();
    }
}

void EditorMenuPanel::SetSaveModalVisibility(bool visible) {
    m_SaveModalVisible = visible;
}
