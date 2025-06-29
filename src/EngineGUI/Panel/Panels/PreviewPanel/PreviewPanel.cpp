#include "PreviewPanel.h"

PreviewPanel::PreviewPanel() 
{

	SetName("Preview");

}

void PreviewPanel::Draw()
{
	if (!m_Visible)
	{
		if (m_TextureID != 0)
		{
            glDeleteTextures(1, &m_TextureID);
            m_TextureID = 0;
		}
        return;
	}

	ImGui::SetNextWindowSizeConstraints(ImVec2(500, 500), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowDockID(ImGui::GetID("LeftDockSpace"), ImGuiCond_FirstUseEver);
    ImGui::Begin(GetName().c_str(), &m_Visible, ImGuiWindowFlags_AlwaysAutoResize);

	if (m_PreviewType == PreviewType::Image)
	{
        int width, height;
        glBindTexture(GL_TEXTURE_2D, m_TextureID);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
        glBindTexture(GL_TEXTURE_2D, 0);
        float scale = 1.0f;
        float maxWidth = 500;
        float maxHeight = 500;
        if (width > maxWidth) {
            scale = maxWidth / width;
        }
        ImGui::Image((ImTextureID) m_TextureID, ImVec2(width * scale, height * scale), ImVec2(0, 1), ImVec2(1, 0));
	}


	ImGui::End();
}

void PreviewPanel::SetImage(GLuint textureID) 
{
	m_TextureID = textureID; 
}

void PreviewPanel::Open() { m_Visible = true; }
