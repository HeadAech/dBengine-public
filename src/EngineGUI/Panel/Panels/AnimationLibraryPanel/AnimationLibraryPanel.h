#ifndef ANIMATION_LIBRARY_PANEL_H
#define ANIMATION_LIBRARY_PANEL_H

#include "EngineGUI/Panel/Panel.h"

class AnimationLibraryPanel : public Panel {

	bool m_Visible = false;

	public:
		AnimationLibraryPanel();
        void Draw() override;

		void Open();
        void Close();
};

#endif // !ANIMATION_LIBRARY_PANEL_H
