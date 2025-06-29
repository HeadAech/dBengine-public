#ifndef EDITOR_MENU_PANEL_H
#define EDITOR_MENU_PANEL_H

#include "../../Panel.h"
#include "../SettingsPanel/SettingsPanel.h"
#include "Scene/Scene.h"

class EditorMenuPanel : public Panel {
    bool m_LoadModalVisible = false;
    bool m_LoadConflictPopupVisible = false;
    bool m_SaveModalVisible = false;
    bool m_SaveConflictPopupVisible = false;
    
    std::string m_PathOfSavedScene;
    std::string m_PathOfLoadedScene;

	public:

		EditorMenuPanel();
        void Draw() override;
        void DrawSaveModal();
        void DrawLoadModal();
        void SetSaveModalVisibility(bool visible);
};

#endif // !EDITOR_MENU_PANEL_H
