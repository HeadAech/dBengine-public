#include "Components/AISystem/NavigationTarget/NavigationTarget.h"

NavigationTarget::NavigationTarget() {
    name = "NavigationTarget";
    this->targetPosition = glm::vec3(0.0f, 0.0f, 0.0f);
}

NavigationTarget::~NavigationTarget() { }

void NavigationTarget::Update(float deltaTime) {
    targetPosition = this->gameObject->transform.GetLocalPosition(); 
}

void NavigationTarget::SetPosition(glm::vec3 pos) {
    targetPosition = pos;
}

glm::vec3 NavigationTarget::GetPosition() { return targetPosition; }
