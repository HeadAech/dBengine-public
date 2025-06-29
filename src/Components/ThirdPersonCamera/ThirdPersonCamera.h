#ifndef THIRD_PERSON_CAMERA_H
#define THIRD_PERSON_CAMERA_H

#include "Components/Camera/Camera.h"

class ThirdPersonCamera : public Camera {
public:
    ThirdPersonCamera(float aspectRatio);
    ~ThirdPersonCamera() override = default;

    void Update(float deltaTime) override;

    void SetTarget(GameObject *target);
    GameObject *GetTarget() const { return m_target; }

    float orbitDistance = 20.0f;
    float orbitYaw = 0.0f;
    float orbitPitch = 20.0f; 
    float targetHeightOffset = 1.6f; 

    float minOrbitPitch = 0.0f;
    float maxOrbitPitch = 60.0f;
    float minDistance = 1.0f;
    float maxDistance = 15.0f;
    float positionSmoothing = 0.0f;
    float rotationSmoothing = 0.0f;
    glm::vec3 Pos; 

private:
    GameObject *m_target = nullptr;

    void updateOrbitPosition();

    glm::vec3 sphericalToCartesian(float distance, float yaw, float pitch) const;
    glm::vec3 getTargetPosition() const;
    void updateCameraRotation(float deltaTime, const glm::vec3 &targetPos);

    float lerpAngle(float current, float target, float t) const;
    float normalizeAngle(float angle) const;
};

#endif // THIRD_PERSON_CAMERA_H
