//
// Created by Hubert Klonowski on 14/03/2025.
//

#ifndef TRANSFORM_H
#define TRANSFORM_H
#include "Component/Component.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>


class Transform : public Component {

public:
    glm::vec3 position = glm::vec3(0, 0, 0);
    glm::quat rotation = glm::quat(1.0f,0.f, 0.f, 0.f);
    glm::vec3 scale = glm::vec3(1.0f);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 globalPosition = glm::vec3(0, 0, 0);

    bool m_isDirty = true;

    Transform();
    ~Transform() = default;

    glm::mat4 GetLocalModelMatrix();
    void ComputeModelMatrix();
    void ComputeModelMatrix(const glm::mat4& parentGlobalModelMatrix);
    void SetLocalPosition(const glm::vec3& localPosition);
    void SetScale(const glm::vec3& newScale);
    void SetEulerRotation(const glm::vec3& newEulerRotation);
    glm::vec3 GetEulerRotation();
    const glm::quat& GetQuatRotation();
    const glm::vec3& GetLocalPosition();
    const glm::vec3& GetScale();
    const glm::mat4& GetModelMatrix() const;
    const void SetModelMatrix(glm::mat4& m);
    bool IsDirty();
    const glm::vec3& GetGlobalPosition();
    void SetGlobalPosition(const glm::vec3 &newGlobalPosition);
    glm::mat4 GetRotationMatrix() const;
    void MoveLocalPosition(const glm::vec3& offset);
    void Rotate(glm::vec3 offset);
    void SetQuatRotation(const glm::quat &newQuatRotation);
    glm::vec3 GetForward() const;
    glm::vec3 GetUp() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetBackward() const;

    glm::vec3 GetGlobalScale() const;



    
};



#endif //TRANSFORM_H
