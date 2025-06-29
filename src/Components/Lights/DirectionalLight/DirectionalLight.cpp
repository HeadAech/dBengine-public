#include "DirectionalLight.h"
#include "Helpers/Util.h"
#include "GameObject/GameObject.h"
#include <Signal/Signals.h>

DirectionalLight::DirectionalLight() 
{ 
	name = "Directional Light"; 
    icon = ICON_FA_SUN_O;
	SetAmbient({0.05f, 0.05f, 0.05f});
    SetDiffuse({0.4f, 0.4f, 0.4f});
    SetSpecular({0.5f, 0.5f, 0.5f});
    SetIntensity(1.5f);

}

glm::vec3 DirectionalLight::GetDirection() { return this->direction; }

void DirectionalLight::SetDirection(glm::vec3 newDirection) 
{ 
    this->direction = newDirection; 
    propertyWatch.Direction = true;
}

void DirectionalLight::Update(float deltaTime) {

    glm::vec3 eulerRot = gameObject->transform.GetEulerRotation();

    //SetDirection(Util::GetDirectionFromEulerAngles(eulerRot.x, eulerRot.y, eulerRot.z));
    SetDirection(Util::GetForwardDirection(gameObject->transform.GetQuatRotation()));
}

