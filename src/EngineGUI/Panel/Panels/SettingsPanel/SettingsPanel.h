#ifndef SETTINGS_PANEL_H
#define SETTINGS_PANEL_H

#include "../../Panel.h"

class SettingsPanel : public Panel {

	bool m_IsVisible = false;

	public:

		SettingsPanel();
        void Draw() override;

		void Open();
        void Close();
};

#endif // !SETTINGS_PANEL_H
