#ifndef INSPECTOR_PANEL_H
#define INSPECTOR_PANEL_H

#include "../../Panel.h"
#include <Component/Component.h>
#include "Components/LuaComponent/LuaComponent.h"
#include "Components/TextRenderer/TextRenderer.h"
#include "Components/WorldEnvironment/WorldEnvironment.h"
#include "Components/Lights/Light.h"
#include "Components/Lights/LocalLight.h"
#include "Components/Lights/SpotLight/SpotLight.h"
#include "Components/MeshInstance/MeshInstance.h"
#include "Components/Animator/Animator.h"
#include "AddComponentPanel/AddComponentPanel.h"
#include "Components/Particles/ParticleSystem.h"

#include "MaterialPanel/MaterialPanel.h"
#include <Components/Control/Control.h>
#include "Components/Tag/Tag.h"

#include <Components/AISystem/AISystem.h>

class ThirdPersonCamera;

class InspectorPanel : public Panel {
    bool useLerp = false;

	void drawComponentSceneTree(Component *component);


	void drawTextRendererComponentPanel(TextRenderer *textRenderer);
	void drawLuaComponentPanel(LuaComponent *luaComponent);

	void drawLightComponentPanel(Light *lightComponent);
    void drawLocalLightComponentPanel(LocalLight *localLightComponent);
	void drawSpotLightComponentPanel(SpotLight *spotLightComponent);

    void drawWorldEnvComponentPanel(WorldEnvironment *worldEnvComponent);

	void drawMeshInstanceComponentPanel(MeshInstance *meshInstanceComponent);
    void drawAnimatorComponentPanel(Animator *pAnimator);

	void drawCameraPanel(Camera *camera);

	void drawParticleSystemPanel(ParticleSystem *pParticleSystem);

	void drawControlPanel(UI::Control* pControl);
    void drawTimerPanel(Timer* pTimer);

	void drawThirdPersonCameraPanel(ThirdPersonCamera *tppCamera);

	void drawAudioSourcePanel(AudioSource *audioSource);

	void drawTagPanel(Tag *tag);

	void drawAISystemPanel(AISystem *aisystem);
    void drawAIAgentPanel(AIAgent *agent);
    void drawNavMeshPanel(NavigationMesh *navMesh);

	AddComponentPanel m_AddComponentPanel;
	MaterialPanel m_MaterialPanel;

	

	public:
		InspectorPanel();
		void Draw() override;
};

#endif // !INSPECTOR_PANEL_H
