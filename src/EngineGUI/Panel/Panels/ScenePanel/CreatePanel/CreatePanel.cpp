#include "CreatePanel.h"
#include <imgui_internal.h>
#include <vector>
#include <dBrender/dBrender.h>
#include <Signal/Signals.h>
#include <Helpers/fonts/IconsFontAwesome4.h>
#include <Helpers/Colors/Colors.h>
#include <EngineGUI/EngineGUI.h>
#include <ImGuiFileDialog.h>


CreatePanel::CreatePanel() { SetName("Create"); }

void CreatePanel::Draw() {
    EngineGUI &gui = EngineGUI::GetInstance();
    GameObject* gameObject = gui.selectedGameObject;
    if (m_IsVisible) {
        ImGui::OpenPopup("Create");
        m_IsVisible = false;
    }

	if (ImGui::BeginPopupModal("Create", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        char nameBuffer[256];
        std::strncpy(nameBuffer, m_NameOfCreatedModel.c_str(), sizeof(nameBuffer));
        ImGui::Text("Name");
        if (ImGui::InputText("##nameCreate", nameBuffer, sizeof(nameBuffer))) {
            m_NameOfCreatedModel = nameBuffer;
        }

        ImGui::Text("Model Path: ");
        ImGui::PushItemWidth(-1);
        ImGui::BeginGroup();

        if (ImGui::Button(ICON_FA_FOLDER "")) {
            IGFD::FileDialogConfig config;
            config.path = "./res/models";
            ImGuiFileDialog::Instance()->OpenDialog("Choose Model", "Select Model File...", "Model files (*.obj *.fbx){.obj,.fbx}", config);
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Choose a path to load the model...");
        }
        ImGui::SameLine();
        ImGui::Text("%s", m_PathToCreateModel.empty() ? "No model selected" : m_PathToCreateModel.c_str());

        ImGui::EndGroup();
        ImGui::PopItemWidth();

        // Handle file dialog
        if (ImGuiFileDialog::Instance()->Display("Choose Model", NULL, ImVec2(600, 300))) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                m_PathToCreateModel = Util::getRelativePath(ImGuiFileDialog::Instance()->GetFilePathName()).string();
            }
            ImGuiFileDialog::Instance()->Close();
        }
       

        ImGui::Text("Position");
        ImGui::DragFloat3("##Pos", (float *) &m_PosOfCreatedModel);
        ImGui::SameLine();
        if (ImGui::Button("Reset")) {
            m_PosOfCreatedModel = {0, 0, 0};
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        float windowWidth = ImGui::GetWindowSize().x;
        float buttonWidth = 150.0f; // Or calculate via ImGui::CalcTextSize + padding
        float buttonX = (windowWidth - buttonWidth) * 0.5f;

        ImGui::SetCursorPosX(buttonX);
   
        ImGui::BeginGroup();

        if (Panel::ColoredButton("Instantiate", Colors::DarkGreen, ImVec2(buttonWidth, 0))) {
            if (m_PathToCreateModel.empty()) {
                std::cerr << "No path to create model!" << std::endl;
            } else {
                Signals::InstantiateGameObject.emit(m_NameOfCreatedModel, m_PathToCreateModel, m_PosOfCreatedModel, gameObject);
            }
            ImGuiFileDialog::Instance()->Close();
            ImGui::CloseCurrentPopup();
        }

        if (Panel::ColoredButton("Cancel", Colors::DarkRed, ImVec2(buttonWidth, 0))) {
            ImGuiFileDialog::Instance()->Close();
            ImGui::CloseCurrentPopup();
        }


        ImGui::EndGroup();
	
        ImGui::EndPopup();
	}
}

void CreatePanel::Open() { m_IsVisible = true; }
