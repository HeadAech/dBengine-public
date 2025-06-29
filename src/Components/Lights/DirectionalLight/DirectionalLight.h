#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "../Light.h"

class DirectionalLight : public Light {

	

public:

	glm::vec3 direction;

    DirectionalLight();
    ~DirectionalLight() = default;
	
	glm::vec3 GetDirection();
    void SetDirection(glm::vec3 newDirection);

	void Update(float deltaTime);


};

#endif // !DIRECTIONAL_LIGHT_H