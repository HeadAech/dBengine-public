#ifndef PREVIEW_PANEL_H
#define PREVIEW_PANEL_H

#include "EngineGUI/Panel/Panel.h"
#include <glad/glad.h>

enum class PreviewType
{
	Image,
	Text
};

class PreviewPanel : public Panel
{

    PreviewType m_PreviewType = PreviewType::Image;

	bool m_Visible = false;

	GLuint m_TextureID;

	public:
		
		PreviewPanel();
        void Draw() override;

		void SetImage(GLuint textureID);

		void Open();
};

#endif // !PREVIEW_PANEL_H
