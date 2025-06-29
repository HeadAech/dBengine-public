#ifndef APPEND_PANEL_H
#define APPEND_PANEL_H

#include "../../../Panel.h"
#include "glm/glm.hpp"

class GameObject;

class AppendPanel : public Panel {

    bool m_IsVisible = false;
    GameObject *m_gameObject = nullptr;

    std::string m_PathOfAppendedScene;

public:
    AppendPanel();
    void Draw() override;

    void Open(GameObject* gameObject);

};

#endif // !APPEND_PANEL_H
