#include <Helpers/Colors/Colors.h>
#include <Helpers/fonts/IconsFontAwesome4.h>
#include <Signal/Signals.h>
#include <dBrender/dBrender.h>
#include <imgui_internal.h>
#include <vector>
#include "AppendPanel.h"
#include <ImGuiFileDialog.h>
#include <Helpers/Util.h>

AppendPanel::AppendPanel() { SetName("Append Scene"); }

void AppendPanel::Draw() {

    if (m_IsVisible) {
        ImGui::OpenPopup("Append Scene");
        m_IsVisible = false;
    }

    if (ImGui::BeginPopupModal("Append Scene", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Path: ");

        ImGui::PushItemWidth(-1);
        ImGui::BeginGroup();

        if (ImGui::Button(ICON_FA_FOLDER "")) 
        {
            IGFD::FileDialogConfig config;
            config.path = "./res/scenes";
            ImGuiFileDialog::Instance()->OpenDialog("Append Scene", "Append Scene...", "Scene files (*.scene *.yaml){.scene,.fbx}", config);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Choose a path to load the scene...");
        }
        ImGui::SameLine();
        ImGui::Text("%s", m_PathOfAppendedScene.empty() ? "No scene selected" : m_PathOfAppendedScene.c_str());

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

        ImGui::BeginDisabled(m_PathOfAppendedScene.empty());
        if (Panel::ColoredButton("Append Scene", Colors::DarkGreen, ImVec2(buttonWidth, 0))) {
            if (m_PathOfAppendedScene.empty()) {
                std::cerr << "No path to append scene from!" << std::endl;
            } else {
                Signals::FileToGameObject.emit(m_PathOfAppendedScene, m_gameObject);
                m_gameObject = nullptr;
            }
            ImGuiFileDialog::Instance()->Close();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        if (Panel::ColoredButton("Cancel", Colors::DarkRed, ImVec2(buttonWidth, 0))) {
            ImGuiFileDialog::Instance()->Close();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndGroup();

        if (ImGuiFileDialog::Instance()->Display("Append Scene", NULL, ImVec2(600, 300))) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                m_PathOfAppendedScene = Util::getRelativePath(ImGuiFileDialog::Instance()->GetFilePathName()).string();
            }
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::EndPopup();
    }

    
}

void AppendPanel::Open(GameObject *gameObject) { 
    m_IsVisible = true; 
    this->m_gameObject = gameObject;
}

