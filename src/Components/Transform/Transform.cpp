//
// Created by Hubert Klonowski on 14/03/2025.
//

#include "Transform.h"

Transform::Transform() : position(0.0f), rotation(1.0f,0.0f,0.0f,0.0f), scale(1.0f) {
    name = "Transform";
    icon = ICON_FA_ARROWS;
}

glm::mat4 Transform::GetLocalModelMatrix() {
    const glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
    // translation * rotation * scale (also know as TRS matrix)
    return glm::translate(glm::mat4(1.0f), position) *
                rotationMatrix *
                glm::scale(glm::mat4(1.0f), scale);
}

void Transform::ComputeModelMatrix() {
    modelMatrix = GetLocalModelMatrix();
    m_isDirty = false;
    globalPosition = position;
}

void Transform::ComputeModelMatrix(const glm::mat4 &parentGlobalModelMatrix) {
    modelMatrix = parentGlobalModelMatrix * GetLocalModelMatrix();
    m_isDirty = false;
    globalPosition = glm::vec3(parentGlobalModelMatrix * glm::vec4(position, 1.0f));
}

void Transform::SetLocalPosition(const glm::vec3 &newPosition) {
    position = newPosition;
    m_isDirty = true;
}

void Transform::SetScale(const glm::vec3 &newScale) {
    scale = newScale;
    m_isDirty = true;
}

void Transform::SetEulerRotation(const glm::vec3 &newEulerRotation) {
    rotation = glm::quat(glm::radians(newEulerRotation));
    rotation = glm::normalize(rotation); // Normalize quaternion -- pervent that weird rotation stuff.
    m_isDirty = true;
}

void Transform::SetQuatRotation(const glm::quat &newQuatRotation) {
    rotation = newQuatRotation;
    m_isDirty = true;
}

glm::vec3 Transform::GetEulerRotation() {
    glm::vec3 euler = glm::degrees(glm::eulerAngles(rotation));
    // Normalize euler
    euler.x = fmod(euler.x + 180.0f, 360.0f) - 180.0f;
    euler.y = fmod(euler.y + 180.0f, 360.0f) - 180.0f;
    euler.z = fmod(euler.z + 180.0f, 360.0f) - 180.0f;
    return euler;
}

const glm::quat &Transform::GetQuatRotation() { 
    return rotation; 
}

const glm::vec3 &Transform::GetLocalPosition() {
    return position;
}

const glm::vec3 &Transform::GetScale() {
    return scale;
}

const glm::mat4 &Transform::GetModelMatrix() const{
    return modelMatrix;
}

const glm::vec3 &Transform::GetGlobalPosition() {
    return globalPosition;
}

bool Transform::IsDirty() {
    return m_isDirty;
}

void Transform::MoveLocalPosition(const glm::vec3 &offset) {
    this->SetLocalPosition(this->GetLocalPosition() + offset);
}

void Transform::Rotate(glm::vec3 offset) {
    //this->SetEulerRotation(this->GetEulerRotation() + offset);

    glm::quat rotOffset = glm::quat(glm::radians(offset));
    this->rotation = rotOffset * this->rotation;
    this->rotation = glm::normalize(this->rotation);
    m_isDirty = true;

}

const void Transform::SetModelMatrix(glm::mat4 &m) {
    modelMatrix = m;
    m_isDirty = false;
}

void Transform::SetGlobalPosition(const glm::vec3 &newGlobalPosition) {
    globalPosition = newGlobalPosition;
    m_isDirty = true;
}

glm::mat4 Transform::GetRotationMatrix() const {
    //return glm::yawPitchRoll(glm::radians(rotation.y), glm::radians(rotation.x), glm::radians(rotation.z));
    return glm::mat4_cast(rotation);
}

glm::vec3 Transform::GetForward() const {
    // return glm::normalize(rotation * glm::vec3(0.f, 0.f, 1.f));
    return -modelMatrix[2];
}

glm::vec3 Transform::GetUp() const {
    //return glm::normalize(rotation * glm::vec3(0.f, 1.f, 0.f));
    return modelMatrix[1];
}

glm::vec3 Transform::GetRight() const { return modelMatrix[0]; }

glm::vec3 Transform::GetBackward() const { return modelMatrix[2]; }

glm::vec3 Transform::GetGlobalScale() const {
    return {glm::length(GetRight()), glm::length(GetUp()), glm::length(GetBackward())};
}
