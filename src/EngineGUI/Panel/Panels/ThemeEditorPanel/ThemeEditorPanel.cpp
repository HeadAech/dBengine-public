#include "ThemeEditorPanel.h"

ThemeEditorPanel::ThemeEditorPanel() 
{

	SetName("Theme Editor");

}

void ThemeEditorPanel::Draw()
{

	if (!m_Visible)
        return;

	ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_Appearing);
	ImGui::Begin("Theme Editor", &m_Visible);

	ImGuiStyle &myStyle = ImGui::GetStyle();
    ImGui::ShowStyleEditor(&myStyle);

	ImGui::End();

}


void ThemeEditorPanel::Open() { m_Visible = true; }

void ThemeEditorPanel::Close() { m_Visible = false; }

