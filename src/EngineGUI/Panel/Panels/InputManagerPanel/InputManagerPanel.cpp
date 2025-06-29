#include "InputManagerPanel.h"
#include <InputManager/Input.h>
#include <Helpers/fonts/IconsFontAwesome4.h>
#include <Helpers/Colors/Colors.h>

InputManagerPanel::InputManagerPanel() {

	SetName("Input Manager");

}

void InputManagerPanel::Draw() { 

	if (!m_Visible)
        return;

	ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Appearing);
    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 300), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin((ICON_FA_KEYBOARD_O " " + GetName()).c_str(), &m_Visible);
		
	if (ImGui::BeginTable("InputTable", 4, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 40.0f);
        ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("##buttons", ImGuiTableColumnFlags_WidthFixed, 55.0f);
        ImGui::TableHeadersRow();

        for (const auto &[action, key]: Input::GetInstance().GetActions()) {
            ImGui::PushID((action + std::to_string(key)).c_str());

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            
            ImGui::Text(action.c_str());
            ImGui::TableNextColumn();

            const char *keyName = glfwGetKeyName(key, 0);
            ImGui::Text(keyName == nullptr ? std::to_string(key).c_str() : keyName);

            ImGui::TableNextColumn();
            if (Input::GetInstance().IsActionPressed(action)) {
                Panel::ColoredText("Pressed", Colors::DarkGreen);
            } else {
                Panel::ColoredText("Not Pressed", Colors::DarkRed);
            }

            ImGui::TableNextColumn();
                
            if (ImGui::Button(ICON_FA_PENCIL "")) {
                // edit logic
            }
            ImGui::SameLine();
            if (Panel::ColoredButton(ICON_FA_TRASH_O, Colors::DarkRed)) {
                // remove logic
            }

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

	ImGui::End();

}

void InputManagerPanel::Open() { m_Visible = true; }

void InputManagerPanel::Close() { m_Visible = false; }
