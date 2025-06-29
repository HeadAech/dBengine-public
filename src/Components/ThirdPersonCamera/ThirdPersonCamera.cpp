#include "ThirdPersonCamera.h"
#include <glm/gtc/constants.hpp>
#include "GameObject/GameObject.h"
#include "dBengine/EngineDebug/EngineDebug.h"

ThirdPersonCamera::ThirdPersonCamera(float aspectRatio) : Camera(aspectRatio) {
    name = "Third Person Camera";
    Pos = glm::vec3(0);
    
}

void ThirdPersonCamera::Update(float deltaTime) {
    if (!m_target) {
        Camera::Update(deltaTime);
        return;
    }
    glm::vec3 targetWorldPos = getTargetPosition();

    glm::vec3 desiredCameraPos = targetWorldPos + sphericalToCartesian(orbitDistance, orbitYaw, orbitPitch);

    if (positionSmoothing <= 0.0f || deltaTime <= 0.0f) {
        Pos = desiredCameraPos; 
    } else {
        float t = 1.0f - exp(-positionSmoothing * deltaTime);
        Pos = glm::mix(Pos, desiredCameraPos, t);
    }
    gameObject->transform.SetLocalPosition(Pos);

    updateCameraRotation(deltaTime, targetWorldPos);
}

void ThirdPersonCamera::SetTarget(GameObject *target) {
    m_target = target;

    if (m_target) {
        glm::vec3 targetPos = getTargetPosition();
        glm::vec3 currentPos = gameObject->transform.GetGlobalPosition();
        glm::vec3 offset = currentPos - targetPos;

        //orbitDistance = glm::length(offset);

        if (orbitDistance > 0.1f) {
            orbitYaw = glm::degrees(atan2(offset.x, offset.z));
            orbitPitch = glm::degrees(asin(offset.y / orbitDistance));
        }
        orbitPitch = glm::clamp(orbitPitch, minOrbitPitch, maxOrbitPitch);
    }
}

void ThirdPersonCamera::updateOrbitPosition() {
    if (!m_target)
        return;

    glm::vec3 offset = sphericalToCartesian(orbitDistance, orbitYaw, orbitPitch);
}

void ThirdPersonCamera::updateCameraRotation(float deltaTime, const glm::vec3 &targetPos) {
    glm::vec3 direction = glm::normalize(targetPos - Pos);

    float targetYaw = glm::degrees(atan2(direction.x, direction.z));
    float targetPitch = glm::degrees(asin(-direction.y));
    targetPitch = glm::clamp(targetPitch, -89.0f, 89.0f);

    if (rotationSmoothing <= 0.0f || deltaTime <= 0.0f) {
        Yaw = targetYaw;
        Pitch = targetPitch;
    } else {
        float t = 1.0f - exp(-rotationSmoothing * deltaTime);
        Yaw = lerpAngle(Yaw, targetYaw, t);
        Pitch = glm::mix(Pitch, targetPitch, t);
    }
    UpdateCameraVectors();
}
float ThirdPersonCamera::lerpAngle(float current, float target, float t) const {
    current = normalizeAngle(current);
    target = normalizeAngle(target);

    float diff = normalizeAngle(target - current);

    return normalizeAngle(current + diff * t);
}

float ThirdPersonCamera::normalizeAngle(float angle) const {
    while (angle > 180.0f)
        angle -= 360.0f;
    while (angle < -180.0f)
        angle += 360.0f;
    return angle;
}

glm::vec3 ThirdPersonCamera::sphericalToCartesian(float distance, float yaw, float pitch) const {
    float radYaw = glm::radians(yaw);
    float radPitch = glm::radians(pitch);

    glm::vec3 offset;
    offset.x = distance * cos(radPitch) * sin(radYaw);
    offset.y = distance * sin(radPitch);
    offset.z = distance * cos(radPitch) * cos(radYaw);

    return offset;
}

glm::vec3 ThirdPersonCamera::getTargetPosition() const {
    if (!m_target)
        return glm::vec3(0.0f);

    glm::vec3 targetPos = m_target->transform.GetGlobalPosition();
    targetPos.y += targetHeightOffset;

    return targetPos;
}
