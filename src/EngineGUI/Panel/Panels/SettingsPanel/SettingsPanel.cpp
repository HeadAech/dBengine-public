#include "SettingsPanel.h"
#include <imgui_internal.h>
#include <dBengine/EngineSettings/EngineSettings.h>
#include <vector>
#include <Helpers/fonts/IconsFontAwesome4.h>
#include <EngineGUI/EngineGUI.h>
#include <Signal/Signals.h>

SettingsPanel::SettingsPanel() { SetName("Settings"); }

void SettingsPanel::Draw() {
    EngineGUI &gui = EngineGUI::GetInstance();

    if (m_IsVisible) {
        ImGui::OpenPopup("Engine Settings");
        m_IsVisible = false;
    }   

    if (ImGui::BeginPopupModal("Engine Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        EngineSettings &engineSettings = EngineSettings::GetEngineSettings();
        bool vsyncEnabled = engineSettings.m_vsyncEnabled;

        ImGui::Text(ICON_FA_PAINT_BRUSH " Editor Theme");

        std::vector<std::string> themes = gui.theme.themes;
        int selectedTheme = engineSettings.selectedTheme;

        if (ImGui::BeginCombo("##ThemeCombo", themes.at(selectedTheme).c_str())) {

            for (int i = 0; i < themes.size(); i++) {
                bool isSelected = (selectedTheme == i);
                if (ImGui::Selectable(themes[i].c_str(), isSelected)) {
                    selectedTheme = i;
                    engineSettings.SetSelectedTheme(i);
                    gui.theme.SetTheme(i);
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::Checkbox("VSync", &vsyncEnabled)) {
            engineSettings.SetVSync(vsyncEnabled);
            Signals::EngineSettings_ChangedVSync.emit();
        }

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::Text(" -- Anti Aliasing -- ");
        /*if (ImGui::Checkbox("MSAA", &engineSettings.m_enableMSAA)) {
            if (engineSettings.m_enableMSAA) {
                dBrender::GetInstance().EnableMSAA();
            } else {
                dBrender::GetInstance().DisableMSAA();
            }
        }*/

        std::vector<std::string> msaaOptions = {"Disabled", "2x MSAA", "4x MSAA", "8x MSAA", "16x MSAA"};
        static int selectedMSAA = 0;

        if (engineSettings.m_MSAAsamples == 2) {
            selectedMSAA = 1;
        } else if (engineSettings.m_MSAAsamples == 4) {
            selectedMSAA = 2;
        } else if (engineSettings.m_MSAAsamples == 8) {
            selectedMSAA = 3;
        } else if (engineSettings.m_MSAAsamples == 16) {
            selectedMSAA = 4;
        } else {
            selectedMSAA = 0;
        }


        if (ImGui::BeginCombo("##MSAACombo", msaaOptions.at(selectedMSAA).c_str())) {
            for (int i = 0; i < msaaOptions.size(); i++) {
                bool isSelected = (selectedMSAA == i);
                if (ImGui::Selectable(msaaOptions[i].c_str(), isSelected)) {
                    selectedMSAA = i;
                    engineSettings.m_enableMSAA = selectedMSAA != 0;
                    if (engineSettings.m_enableMSAA) {
                        dBrender::GetInstance().EnableMSAA();
                    } else {
                        dBrender::GetInstance().DisableMSAA();
                        Signals::Engine_SetMSAASamples.emit(0);
                        engineSettings.m_MSAAsamples = 0;
                    }
                    switch (selectedMSAA) {
                        case 1:
                            Signals::Engine_SetMSAASamples.emit(2);
                            engineSettings.m_MSAAsamples = 2;
                            break;
                        case 2:
                            Signals::Engine_SetMSAASamples.emit(4);
                            engineSettings.m_MSAAsamples = 4;
                            break;
                        case 3:
                            Signals::Engine_SetMSAASamples.emit(8);
                            engineSettings.m_MSAAsamples = 8;
                            break;
                        case 4:
                            Signals::Engine_SetMSAASamples.emit(16);
                            engineSettings.m_MSAAsamples = 16;
                            break;
                    }
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing();

        ImGui::SeparatorText("Shader Settings");

        ImGui::Text("HDR Exposure:");
        ImGui::SameLine();
        static float exposure = 1.0f;
        if (ImGui::DragFloat("##HDRExposureDrag", &exposure, 0.1f))
        {
            dBrender::GetInstance().hdrShader->Use();
            dBrender::GetInstance().hdrShader->SetFloat("exposure", exposure);
        }

        ImGui::Text("Shadow Cutoff Distance:");
        ImGui::PushItemWidth(-1);
        ImGui::DragFloat("##ShaderCutoffShadow", &engineSettings.shadowCutoffStart, 0.5f, 30.0f, 200.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::Separator();
        ImGui::Spacing();

        float windowWidth = ImGui::GetWindowSize().x;
        float buttonWidth = 80.0f; // Or calculate via ImGui::CalcTextSize + padding
        float buttonX = (windowWidth - buttonWidth) * 0.5f;

        ImGui::SetCursorPosX(buttonX);
        if (ImGui::Button(ICON_FA_CHECK " Done", ImVec2(buttonWidth, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void SettingsPanel::Open() { 
    m_IsVisible = true;
}

void SettingsPanel::Close() { 
    if (!m_IsVisible)
        return;
    m_IsVisible = false;
    ImGui::CloseCurrentPopup(); 
}
