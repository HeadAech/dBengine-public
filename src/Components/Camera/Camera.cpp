//
// Created by Hubert Klonowski on 14/03/2025.
//

#include "Camera.h"

#include <iostream>
#include <ostream>

#include "InputManager/Input.h"
#include "Signal/Signals.h"
#include "dBengine/EngineDebug/EngineDebug.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/quaternion.hpp"

Camera::Camera() : Front(glm::vec3(0.0f, 0.0f, -1.0f)), Zoom(ZOOM)
{
    name = "Camera";
    icon = ICON_FA_CAMERA;
    Signals::UpdateCameraVectors.connect(this->GetUUID(), [this]() { UpdateCameraVectors(); });
    Signals::Camera_UpdateAspectRatio.connect(this->GetUUID(),
                                              [this](float newAspectRatio) { this->aspectRatio = newAspectRatio; });
}

Camera::Camera(float aspectRatio) : aspectRatio(aspectRatio), Front(glm::vec3(0.0f, 0.0f, -1.0f)), Zoom(ZOOM) {
    name = "Camera";
    icon = ICON_FA_CAMERA;
    Signals::UpdateCameraVectors.connect(this->GetUUID(), [this]() { UpdateCameraVectors(); });
    Signals::Camera_UpdateAspectRatio.connect(this->GetUUID(), [this](float newAspectRatio) { this->aspectRatio = newAspectRatio; });
}

Camera::~Camera(){
    Signals::UpdateCameraVectors.disconnect(this->GetUUID());
    Signals::Camera_UpdateAspectRatio.disconnect(this->GetUUID());
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(gameObject->transform.position, gameObject->transform.position + Front, Up);
}

void Camera::SetClippingPlanes(float nearPlane, float farPlane) {
    NearPlane = nearPlane;
    FarPlane = farPlane;
}

void Camera::LookAt(glm::vec3 target) {
    glm::vec3 position = gameObject->transform.GetGlobalPosition();
    glm::vec3 front = glm::normalize(target - position);
    if (glm::length(front) < 0.0001f)
        return; 

    // Get Yaw and Pitch from the front vector
    Yaw = glm::degrees(atan2(front.z, front.x));
    Pitch = glm::degrees(asin(front.y));

    // Clamp Pitch to prevent flipping
    if (Pitch > 89.0f)
        Pitch = 89.0f;
    if (Pitch < -89.0f)
        Pitch = -89.0f;

    // quaternion from Yaw and Pitch
    glm::quat yawQuat = glm::angleAxis(glm::radians(Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat pitchQuat = glm::angleAxis(glm::radians(Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat rotation = yawQuat * pitchQuat; // Yaw applied after Pitch [important stuff]

    gameObject->transform.SetQuatRotation(rotation);
    UpdateCameraVectors();
}

void Camera::UpdateCameraVectors() {
    // Construct quaternion from Yaw and Pitch to ensure no Roll
    glm::quat yawQuat = glm::angleAxis(glm::radians(Yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::quat pitchQuat = glm::angleAxis(glm::radians(Pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::quat rotation = yawQuat * pitchQuat; // Yaw after Pitch [important stuff again]

    // Set the transform's rotation
    //gameObject->transform.SetQuatRotation(rotation);

    // Compute Front vector
    glm::vec3 defaultFront(0.0f, 0.0f, 1.0f);
    Front = glm::normalize(glm::rotate(rotation, defaultFront));

    // Compute Right and Up vectors
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

glm::mat4 Camera::GetProjectionMatrix() {
    return glm::perspective(glm::radians(this->Zoom), aspectRatio, NearPlane, FarPlane);
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    // Update Yaw and Pitch
    Yaw += xoffset;
    Pitch -= yoffset; // Invert yoffset for typical FPS controls

    // Constrain Pitch
    if (constrainPitch) {
        if (Pitch > 89.0f)
            Pitch = 89.0f;
        if (Pitch < -89.0f)
            Pitch = -89.0f;
    }

    UpdateCameraVectors(); // Updates quaternion and vectors
}

void Camera::AssignLerpValues(Camera *camera) {
    this->lerpPosition = gameObject->transform.position;
    this->lerpRotation = gameObject->transform.rotation;
    this->lerpFront = Front;
    this->lerpPitch = Pitch;
    this->lerpRight = Right;
    this->lerpUp = Up;
    this->lerpYaw = Yaw;
    this->lerpZoom = Zoom;
    this->isLerping = true;

    this->gameObject->transform.position = camera->gameObject->transform.position;
    this->gameObject->transform.rotation = camera->gameObject->transform.rotation;
    this->Front = camera->Front;
    this->Pitch = camera->Pitch;
    this->Right = camera->Right;
    this->Up = camera->Up;
    this->Yaw = camera->Yaw;
    this->Zoom = camera->Zoom;

    this->isUsed = false;

}

void Camera::ClearLerpValues() {
    
}

void Camera::ProcessLerp(float deltaTime) {
    if (true) {
        this->gameObject->transform.position = glm::mix(gameObject->transform.position, lerpPosition, deltaTime * 2.0f);
        this->gameObject->transform.rotation = glm::mix(gameObject->transform.rotation, lerpRotation, deltaTime * 2.0f);
        this->Yaw = glm::mix(Yaw, lerpYaw, deltaTime * 2.0f);
        this->Pitch = glm::mix(Pitch, lerpPitch, deltaTime * 2.0f);
        this->Zoom = glm::mix(Zoom, lerpZoom, deltaTime * 2.0f);
        this->Front = glm::mix(Front, lerpFront, deltaTime * 2.0f);
        this->Right = glm::mix(Right, glm::normalize(glm::cross(this->Front, this->WorldUp)), deltaTime * 2.0f);
        this->Up = glm::mix(Up, glm::normalize(glm::cross(this->Right, this->Front)), deltaTime * 2.0f);
    }

    bool isPositionClose = glm::all(glm::lessThan(glm::abs(gameObject->transform.position - lerpPosition), glm::vec3(TOLERANCE)));
    bool isRotationClose = glm::dot(gameObject->transform.rotation, lerpRotation) > (1.0f - TOLERANCE);
    bool isYawClose = std::abs(Yaw - lerpYaw) < TOLERANCE;
    bool isPitchClose = std::abs(Pitch - lerpPitch) < TOLERANCE;
    bool isZoomClose = std::abs(Zoom - lerpZoom) < TOLERANCE;
    bool isFrontClose = glm::all(glm::lessThan(glm::abs(Front - lerpFront), glm::vec3(TOLERANCE)));
    bool isRightClose = glm::all(glm::lessThan(glm::abs(Right - glm::normalize(glm::cross(this->Front, this->WorldUp))), glm::vec3(TOLERANCE)));
    bool isUpClose = glm::all(glm::lessThan(glm::abs(Up - glm::normalize(glm::cross(this->Right, this->Front))), glm::vec3(TOLERANCE)));
    if (isPositionClose && isRotationClose && isYawClose && isPitchClose) {
        this->gameObject->transform.position = lerpPosition;
        this->gameObject->transform.rotation = lerpRotation;
        this->Yaw = lerpYaw;
        this->Pitch = lerpPitch;
        this->Zoom = lerpZoom;
        this->Front = lerpFront;
        this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));
        this->Up = glm::normalize(glm::cross(this->Right, this->Front));
        //ClearLerpValues();
        isLerping = false;
        isUsed = true;
    }
}

void Camera::Update(float deltaTime) {

    if(isLerping){
        ProcessLerp(deltaTime);
    }
}