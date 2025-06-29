#ifndef SCENE_PANEL_H
#define SCENE_PANEL_H

#include "../../Panel.h"
#include "Scene/Scene.h"
#include "CreatePanel/CreatePanel.h"
#include "AppendPanel/AppendPanel.h"

class ScenePanel : public Panel {

    void drawSceneTree(Scene *scene);
    void drawGameObjectSceneTree(GameObject *gameObject);

    void drawContextMenu();

    bool m_DeleteConfirmationVisible = false;
    GameObject *p_GameObjectToDelete = nullptr;
    void drawConfirmationDelete();

    char m_NameBuffer[256]; 
    GameObject *p_GameObjectToEdit = nullptr;
    bool m_RenameModalVisible = false;
    void drawRenameModal();

    //error popups
    bool m_NameConflictPopupVisible = false;
    bool m_WarningLoadScenePopup = false;
    void drawNameConflictPopup();

    void drawWarningLoadSceneModal();

    CreatePanel m_CreatePanel;
    AppendPanel m_AppendPanel;

	public:

		ScenePanel();
        void Draw() override;
};

#endif // !SCENE_PANEL_H
