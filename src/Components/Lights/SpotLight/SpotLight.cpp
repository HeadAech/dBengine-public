#include "SpotLight.h"
#include "GameObject/GameObject.h"
#include "Helpers/Util.h"
#include <Signal/Signals.h>

SpotLight::SpotLight() { 
	name = "Spot Light"; 
    icon = ICON_FA_LIGHTBULB_O;
	SetAmbient({0, 0, 0});
    SetDiffuse({1.0f, 0, 0});
    SetIntensity(8);
    SetSpecular({1, 1, 1});
}

glm::vec3 SpotLight::GetDirection() { return this->direction; }

float SpotLight::GetInnerCutOff() { return this->innerCutOff; }

float SpotLight::GetOuterCutOff() { return this->outerCutOff; }

void SpotLight::SetDirection(glm::vec3 newDirection) 
{ 
    this->direction = newDirection; 
    propertyWatch.Direction = true;
}

void SpotLight::SetInnerCutOff(float newCutOff) 
{ 
    this->innerCutOff = newCutOff; 
    propertyWatch.InnerCutOff = true;
}

void SpotLight::SetOuterCutOff(float newCutOff) 
{ 
    this->outerCutOff = newCutOff; 
    propertyWatch.OuterCutOff = true;
}

void SpotLight::Update(float deltaTime) { 
    glm::vec3 eulerRot = gameObject->transform.GetEulerRotation();
   
    SetDirection(Util::GetForwardDirection(gameObject->transform.GetQuatRotation()));
}
