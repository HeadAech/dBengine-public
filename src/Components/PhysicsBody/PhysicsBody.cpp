#include "PhysicsBody.h"
#include "GameObject/GameObject.h"
#include "dBphysics/dBphysics.h"
#include <dBengine/EngineDebug/EngineDebug.h>

PhysicsBody::PhysicsBody(float m, bool gravity) : Component() {
    name = "PhysicsBody";
    icon = ICON_FA_MALE;
    mass = m;
    useGravity = gravity;
    if (mass <= 0.0f || isStatic) {
        isStatic = true;
        invMass = 0.0f;
    } else {
        invMass = 1.0f / mass;
    }
}

void PhysicsBody::ApplyForce(const glm::vec3 &force) {
    if (!isStatic) {
        accumulatedForces += force;
    }
}

void PhysicsBody::Integrate(float deltaTime) {
    if (isStatic || !gameObject || !enabled)
        return;

    glm::vec3 position = gameObject->transform.GetLocalPosition();

    if (useGravity) {
        velocity += dBphysics::GetInstance().GetGravity() * deltaTime;
    }

    glm::vec3 acceleration = accumulatedForces * invMass;
    velocity += acceleration * deltaTime;

   if (linearDamping > 0.0f) {
        float dampingFactor = std::pow(1.0f - linearDamping, deltaTime);
        velocity *= dampingFactor;
    }

    position += velocity * deltaTime;

    gameObject->transform.SetLocalPosition(position);

    accumulatedForces = glm::vec3(0.0f);

    if (isGrounded && glm::abs(velocity.y) < 0.5f) {
        velocity.y = 0.0f;
    }
}

void PhysicsBody::SetStatic(bool static_state) {
    isStatic = static_state;
    if (isStatic) {
        invMass = 0.0f;
        velocity = glm::vec3(0.0f);
        accumulatedForces = glm::vec3(0.0f);
    } else {
        invMass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
    }
}

void PhysicsBody::SetMass(float new_mass) {
    mass = new_mass;
    if (mass <= 0.0f || isStatic) {
        invMass = 0.0f;
    } else {
        invMass = 1.0f / mass;
    }
}
