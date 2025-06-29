//
// Created by Hubert Klonowski on 14/03/2025.
//

#ifndef CAMERA_H
#define CAMERA_H

#include "Component/Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "GameObject/GameObject.h"
#include <glad/glad.h>

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW         =  -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  12.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  70.0f;

const float NORMAL_SPEED = 12.5f;
const float SPRINT_SPEED = 30.0f;

class Camera : public Component {
    const float TOLERANCE = 0.1f;

public:
    void Update(float deltaTime) override;
    // camera Attributes in sake of lerp.
    
    glm::vec3 lerpPosition;
    glm::quat lerpRotation;
    glm::vec3 lerpFront;
    glm::vec3 lerpUp;
    glm::vec3 lerpRight;
    float lerpYaw = YAW;
    float lerpPitch = PITCH;
    float lerpZoom;

     
     
    // camera Attributes
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp = {0, 1, 0};
    // euler Angles
    float Yaw = YAW;
    float Pitch = PITCH;
    // camera options
    float MovementSpeed = SPEED;
    float MouseSensitivity = SENSITIVITY;
    float Zoom;

    float aspectRatio;

    float NearPlane = 0.1f; // Default near plane
    float FarPlane = 400.0f; // Default far plane
    bool isUsed = false;
    bool isLerping = false;

    // follow attributes
    GameObject *followTarget = nullptr;
    glm::vec3 followOffset = glm::vec3(0, 2, -5);
    float followSmoothing = 5.0f;
    bool isFollowing = false;
    float followDistance = 5.0f;

    Camera();
    Camera(float aspectRatio);
    ~Camera();
    void UpdateCameraVectors();

    glm::mat4 GetProjectionMatrix();
    glm::mat4 GetViewMatrix();
    void SetClippingPlanes(float nearPlane, float farPlane);
    void LookAt(glm::vec3 target);
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
    void AssignLerpValues(Camera *camera);
    void ClearLerpValues();
    void ProcessLerp(float deltaTime);

};



#endif //CAMERA_H
