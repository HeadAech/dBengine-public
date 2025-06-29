#ifndef SPOT_LIGHT_H
#define SPOT_LIGHT_H

#include "../LocalLight.h"

class SpotLight : public LocalLight {

    
public:
    glm::vec3 direction;

    float innerCutOff = glm::cos(glm::radians(12.92f));
    float outerCutOff = glm::cos(glm::radians(17.29f));

    SpotLight();
    ~SpotLight() = default;

    void Update(float deltaTime);

    glm::vec3 GetDirection();
    float GetInnerCutOff();
    float GetOuterCutOff();

    void SetDirection(glm::vec3 newDirection);
    void SetInnerCutOff(float newCutOff);
    void SetOuterCutOff(float newCutOff);


};

#endif // !SPOT_LIGHT_H
