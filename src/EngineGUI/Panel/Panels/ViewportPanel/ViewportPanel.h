#ifndef VIEWPORT_PANEL_H
#define VIEWPORT_PANEL_H

#include "../../Panel.h"
#include "Components/Camera/Camera.h"
#include "Scene/Scene.h"	

class ViewportPanel : public Panel {

	bool m_movingCameraInEditor = false;
    //bool m_editorFullscreen = false;
    bool m_viewCubeCameraFlag = false;

	Camera *m_camera;

	Scene *m_Scene;

	public:

		ViewportPanel();

		void Draw() override;

		void SetCamera(Camera *camera);
        void SetScene(Scene *scene);
};

#endif // !VIEWPORT_PANEL_H
