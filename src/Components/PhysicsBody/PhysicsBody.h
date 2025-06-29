#pragma once
#pragma once
#include <glm/glm.hpp>
#include "Component/Component.h"

class GameObject;

class PhysicsBody : public Component {
public:
    float mass = 1.0f;
    float invMass = 1.0f;
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 accumulatedForces = glm::vec3(0.0f);
    bool useGravity = true;
    bool isStatic = false;
    float restitution = 0.1f; 
    float linearDamping = 0.1f;
    bool isGrounded = false;
    glm::vec3 stabilizationPosition = glm::vec3(0.0f);
    PhysicsBody(float mass = 1.0f, bool gravity = true);
    ~PhysicsBody() override = default;

    void ApplyForce(const glm::vec3 &force);
    void Integrate(float deltaTime);
    void SetStatic(bool isStatic);
    void SetMass(float mass);
};
