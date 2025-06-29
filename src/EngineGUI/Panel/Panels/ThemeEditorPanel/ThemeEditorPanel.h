#ifndef THEME_EDITOR_PANEL_H
#define THEME_EDITOR_PANEL_H

#include "EngineGUI/Panel/Panel.h"

class ThemeEditorPanel : public Panel
{

	bool m_Visible = false;

	public:
		
		ThemeEditorPanel();
        void Draw() override;

		void Open();
        void Close();
};

#endif // !THEME_EDITOR_PANEL_H
