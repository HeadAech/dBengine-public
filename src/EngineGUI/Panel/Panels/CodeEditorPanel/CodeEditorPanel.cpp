#include "CodeEditorPanel.h"
#include <imgui_internal.h>
#include <filesystem>
#include <EngineGUI/EngineGUI.h>

CodeEditorPanel::CodeEditorPanel() { SetName("Code Editor"); }

void CodeEditorPanel::Draw() {
    // std::string codeEditorTitle = "Script Editor - " + std::filesystem::path(scriptPath).filename().string();
    // if (isModified) codeEditorTitle += " - Modified";

    EngineGUI &gui = EngineGUI::GetInstance();

    ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Once);
    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 300), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::Begin("Code Editor", &gui.codeEditorEnabled, gui.isModified ? ImGuiWindowFlags_UnsavedDocument : ImGuiWindowFlags_None);
    ImGui::Text(std::filesystem::path(gui.scriptPath).filename().string().c_str());

    const std::string &extension = std::filesystem::path(gui.scriptPath).extension().string();

    ImGui::Spacing();

    ImGui::Text("CTRL + S - Save | CTRL + R - Save & Reload");

    if (extension == ".lua" || extension == ".vert" || extension == ".frag" || extension == ".vs" ||
        extension == ".fs" || extension == ".geom") {
        if (ImGui::Button("Reload")) {
            // reload script
            gui.reloadCode(extension);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        // save script
        gui.saveScript();
    }
    if (gui.isModified) {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Modified!");
        ImGui::SameLine();
        if (ImGui::Button("Revert changes")) {
            std::strncpy(gui.scriptBuffer, gui.originalScript.c_str(), gui.BUF_SIZE - 1);
            gui.scriptBuffer[gui.BUF_SIZE - 1] = '\0';
            gui.isModified = false;
        }
    }

    static ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput;

    ImVec2 availableSize = ImGui::GetContentRegionAvail();
    
    auto &ff_code = gui.GetCodeFontFamily();

    ImGui::PushFont(ff_code.GetFont(REGULAR));
    if (ImGui::InputTextMultiline("##script", gui.scriptBuffer, gui.BUF_SIZE, ImVec2(availableSize.x, availableSize.y),
                                  flags)) {
        // check for changes
        gui.isModified = (gui.originalScript != gui.scriptBuffer);
    }
    ImGui::PopFont();

    ImGui::End();
}
