#ifndef CREATE_PANEL_H
#define CREATE_PANEL_H

#include "../../../Panel.h"
#include "glm/glm.hpp"

class CreatePanel : public Panel {

	bool m_IsVisible = false;

	std::string m_NameOfCreatedModel = "Instance";
    std::string m_PathToCreateModel;
    glm::vec3 m_PosOfCreatedModel = {0, 0, 0};


	public:
		CreatePanel();
        void Draw() override;

		void Open();
};

#endif // !CREATE_PANEL_H
