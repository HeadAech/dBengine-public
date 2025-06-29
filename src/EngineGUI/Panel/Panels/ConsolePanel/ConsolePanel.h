#ifndef CONSOLE_PANEL_H
#define CONSOLE_PANEL_H	

#include "../../Panel.h"

class ConsolePanel : public Panel {

    bool m_ScrollConsoleToBottom = true;

	public:
        ConsolePanel();

        void Draw() override;

};


#endif // !CONSOLE_PANEL_H
