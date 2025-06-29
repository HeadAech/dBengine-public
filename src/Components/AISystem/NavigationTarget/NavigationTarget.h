#ifndef NAVIGATIONTARGET_H
#define NAVIGATIONTARGET_H

#include <glm/glm.hpp>
#include <vector>
#include "Component/Component.h"
#include "GameObject/GameObject.h"
#include "Components/AISystem/AISystem.h"

class NavigationSystem;

class NavigationTarget : public Component {
	public:
        NavigationTarget();
        ~NavigationTarget();
        void Update(float deltaTime); 

        glm::vec3 GetPosition();
        void SetPosition(glm::vec3 pos);
    private:
        glm::vec3 targetPosition;
};

#endif // NAVIGATIONTARGET_H