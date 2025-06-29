#include "ConsolePanel.h"
#include <imgui_internal.h>
#include <dBengine/EngineDebug/EngineDebug.h>
#include "Signal/Signals.h"

ConsolePanel::ConsolePanel() { 
    SetName("Console"); 
    Signals::Console_ScrollToBottom.connect(this->uuid,[this] { m_ScrollConsoleToBottom = true; });
}

void ConsolePanel::Draw() {
    ImGui::SetNextWindowSizeConstraints(ImVec2(400, 150), ImVec2(FLT_MAX, FLT_MAX));

    EngineDebug &engineDebug = EngineDebug::GetInstance();

    ImGui::Begin("Console");

    ImGui::BeginChild("ConsoleScroll", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    for (int i = 0; i < engineDebug.logBuffer.size(); i++) {
        LogLevel level = engineDebug.logBufferLevels.at(i);
        const std::string& log = engineDebug.logBuffer.at(i);
        if (log.empty()) {
            continue; // Skip empty logs
        }
        ImVec4 color;
        switch (level) {
            case LogLevel::INFO:
                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                break;
            case LogLevel::WARNING:
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                break;
            case LogLevel::ERR:
                color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                break;
            case LogLevel::DEBUG:
                color = ImVec4(0.2f, 0.0f, 1.0f, 1.0f);
                break;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(log.c_str());
        ImGui::PopStyleColor();
    }

    if (m_ScrollConsoleToBottom) {
        ImGui::SetScrollHereY(1.0f);
        m_ScrollConsoleToBottom = false;
    }

    ImGui::EndChild();
    ImGui::End();
}
