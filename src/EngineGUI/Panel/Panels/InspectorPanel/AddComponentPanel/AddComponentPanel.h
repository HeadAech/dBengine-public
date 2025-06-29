#ifndef ADD_COMPONENT_PANEL_H
#define ADD_COMPONENT_PANEL_H

#include "EngineGUI/Panel/Panel.h"

class AddComponentPanel : public Panel
{

    bool m_Visible = false;

	public:
        AddComponentPanel();
        void Draw() override;

        void Open();
        void Close();
};

#endif // !ADD_COMPONENT_PANEL_H
