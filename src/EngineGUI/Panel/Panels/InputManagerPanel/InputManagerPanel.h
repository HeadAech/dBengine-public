#ifndef INPUT_MANAGER_PANEL_H
#define INPUT_MANAGER_PANEL_H

#include "../../Panel.h"

class InputManagerPanel : public Panel {

    bool m_Visible = false;

	public:
        InputManagerPanel();
        ~InputManagerPanel() = default;
        void Draw() override;

        void Open();
        void Close();
};

#endif // !INPUT_MANAGER_PANEL_H
