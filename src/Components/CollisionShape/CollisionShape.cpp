//
// Created by Hubert Klonowski on 19/03/2025.
//

#include "CollisionShape.h"
#include <cmath>
#include <vector>
#include "Components/MeshInstance/MeshInstance.h"
#include "GameObject/GameObject.h"
#include "dBrender/dBrender.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <dBphysics/dBphysics.h>

std::vector<CollisionShape *> CollisionShape::colliders;

CollisionShape::CollisionShape(std::string SignalMessage_OnAreaEntered) :
    m_shapeType(ShapeType::BOX), m_boxSize(1.0f, 1.0f, 1.0f), m_capsuleRadius(0.5f), m_capsuleHeight(2.0f),
    m_color(0.0f, 1.0f, 0.0f), m_isVisible(false), m_collisionLayer(1 << 0), m_collisionMask(1 << 0) {
    name = "CollisionShape";
    icon = ICON_FA_STREET_VIEW;
    enabled = true;
    colliders.push_back(this); 
    this->SignalMessage_OnAreaEntered = SignalMessage_OnAreaEntered;
}

CollisionShape::~CollisionShape() {
    auto it = std::find(colliders.begin(), colliders.end(), this);
    if (it != colliders.end()) {
        colliders.erase(it);
    }

    dBphysics::GetInstance().RemoveCollisionShape(this);
}

void CollisionShape::Update(float deltaTime) {}

void CollisionShape::SetBoxSize(const glm::vec3 &size) {
    m_boxSize = size;
    MarkAABBDirty();
}

void CollisionShape::SetCapsuleParams(float radius, float height) {
    m_capsuleRadius = radius;
    m_capsuleHeight = height;
    MarkAABBDirty();
}

void CollisionShape::SetShapeType(ShapeType type) {
    m_shapeType = type;
    MarkAABBDirty();
}

void CollisionShape::SetShapeType(int type) {
    m_shapeType = static_cast<ShapeType>(type);
    MarkAABBDirty();
}

void CollisionShape::SetColor(const glm::vec3 &color) { m_color = color; }

void CollisionShape::SetVisible(bool visible) { m_isVisible = visible; }

static bool GetAllVisible();
static void SetAllVisible(bool visible);

void CollisionShape::Render(Shader *shader) const {
    //return;
    if (!m_isVisible || !enabled || (!gameObject || !gameObject->m_enabled))
        return;

    shader->Use();
    shader->SetVec3("color", m_color);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::vec3 position = gameObject->transform.GetGlobalPosition() + m_positionOffset;
    modelMatrix = glm::translate(modelMatrix, position);
    glm::quat rotation = gameObject->transform.GetQuatRotation();
    modelMatrix = modelMatrix * glm::mat4_cast(rotation);

    GLfloat prevLineWidth;
    glGetFloatv(GL_LINE_WIDTH, &prevLineWidth);
    glLineWidth(2.0f);

    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    glDisable(GL_DEPTH_TEST);

    if (m_shapeType == ShapeType::BOX) {
        static const GLfloat vertices[] = {// Front face
                                           -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f, 0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
                                           // Back face
                                           -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, -0.5f, 0.5f,
                                           -0.5f};

        static const GLuint indices[] = {// Front
                                         0, 1, 1, 2, 2, 3, 3, 0,
                                         // Back
                                         4, 5, 5, 6, 6, 7, 7, 4,
                                         // Connecting sides
                                         0, 4, 1, 5, 2, 6, 3, 7};

        static GLuint VAO = 0, VBO = 0, EBO = 0;

        if (VAO == 0) {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
            glEnableVertexAttribArray(0);
            glBindVertexArray(0);
        }

        glm::mat4 scaledModel = modelMatrix;
        glm::vec3 combinedScale = m_boxSize * gameObject->transform.GetScale();
        scaledModel = glm::scale(scaledModel, combinedScale);
        shader->SetMat4("model", scaledModel);

        glBindVertexArray(VAO);
        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    } else if (m_shapeType == ShapeType::CAPSULE) {

        glm::vec3 objectScale = gameObject->transform.GetScale();

        float scaledRadius = m_capsuleRadius * (objectScale.x + objectScale.z) * 0.5f;
        float scaledHeight = m_capsuleHeight * objectScale.y;
        const int segments = 16;
        const int rings = 8;

        std::vector<GLfloat> vertices;
        std::vector<GLuint> indices;

        for (int i = 0; i < segments; i++) {
            float angle1 = 2.0f * M_PI * (float) i / segments;
            float angle2 = 2.0f * M_PI * (float) (i + 1) / segments;

            float x1 = scaledRadius * cosf(angle1);
            float z1 = scaledRadius * sinf(angle1);

            float x2 = scaledRadius * cosf(angle2);
            float z2 = scaledRadius * sinf(angle2);

            float y_top = (scaledHeight / 2.0f) - scaledRadius;
            vertices.push_back(x1);
            vertices.push_back(y_top);
            vertices.push_back(z1);

            float y_bottom = -y_top;
            vertices.push_back(x1);
            vertices.push_back(y_bottom);
            vertices.push_back(z1);

            indices.push_back(i * 2);
            indices.push_back(i * 2 + 1);

            indices.push_back(i * 2);
            indices.push_back(((i + 1) % segments) * 2);

            indices.push_back(i * 2 + 1);
            indices.push_back(((i + 1) % segments) * 2 + 1);
        }

        int baseIndex = segments * 2;

        for (int r = 1; r <= rings; r++) {
            float phi = M_PI / 2.0f * (float) r / rings;
            float y = (scaledHeight / 2.0f) - scaledRadius + scaledRadius * sinf(phi);
            float ringRadius = scaledRadius * cosf(phi);

            for (int s = 0; s < segments; s++) {
                float theta = 2.0f * M_PI * (float) s / segments;
                float x = ringRadius * cosf(theta);
                float z = ringRadius * sinf(theta);

                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                if (r > 1) {
                    indices.push_back(baseIndex + (r - 2) * segments + s);
                    indices.push_back(baseIndex + (r - 1) * segments + s);
                } else {
                    indices.push_back(s * 2);
                    indices.push_back(baseIndex + s);
                }

                indices.push_back(baseIndex + (r - 1) * segments + s);
                indices.push_back(baseIndex + (r - 1) * segments + ((s + 1) % segments));
            }
        }

        int bottomBase = baseIndex + rings * segments;

        for (int r = 1; r <= rings; r++) {
            float phi = M_PI / 2.0f * (float) (rings - r) / rings;
            float y = -(scaledHeight / 2.0f) + scaledRadius - scaledRadius * sinf(phi);
            float ringRadius = scaledRadius * cosf(phi);

            for (int s = 0; s < segments; s++) {
                float theta = 2.0f * M_PI * (float) s / segments;
                float x = ringRadius * cosf(theta);
                float z = ringRadius * sinf(theta);

                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);

                if (r > 1) {
                    indices.push_back(bottomBase + (r - 2) * segments + s);
                    indices.push_back(bottomBase + (r - 1) * segments + s);
                } else {
                    indices.push_back(s * 2 + 1); 
                    indices.push_back(bottomBase + s);
                }

                indices.push_back(bottomBase + (r - 1) * segments + s);
                indices.push_back(bottomBase + (r - 1) * segments + ((s + 1) % segments));
            }
        }
        GLuint capsuleVAO, capsuleVBO, capsuleEBO;

        glGenVertexArrays(1, &capsuleVAO);
        glGenBuffers(1, &capsuleVBO);
        glGenBuffers(1, &capsuleEBO);

        glBindVertexArray(capsuleVAO);

        glBindBuffer(GL_ARRAY_BUFFER, capsuleVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, capsuleEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(0);

        shader->SetMat4("model", modelMatrix);
        glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);

        glDeleteVertexArrays(1, &capsuleVAO);
        glDeleteBuffers(1, &capsuleVBO);
        glDeleteBuffers(1, &capsuleEBO);

        glBindVertexArray(0);
    }
}

void CollisionShape::RenderColliders(Shader *shader) {
    for (auto collider: colliders) {
        collider->Render(shader);
    }
}

void CollisionShape::SetWorldMin(glm::vec3 worldMin) {
    this->m_WorldMin = worldMin;
}

void CollisionShape::SetWorldMax(glm::vec3 worldMax) {
    this->m_WorldMax = worldMax; }

void CollisionShape::SetIsCollisionArea(bool m_IsCollisionArea) {
    this->m_IsCollisionArea = m_IsCollisionArea; }

void CollisionShape::SetHasEnteredArea(bool m_HasEnteredInArea) {
    this->m_HasEnteredInArea = m_HasEnteredInArea;
}

void CollisionShape::MarkAABBDirty() { m_aabbDirty = true; }

void CollisionShape::UpdateAABB() {
    if (!m_aabbDirty || !gameObject)
        return;

    if (m_shapeType == ShapeType::BOX) {
        glm::vec3 halfSize = m_boxSize * gameObject->transform.GetScale() * 0.5f;
        glm::mat4 transform = gameObject->transform.GetModelMatrix();

        glm::vec3 offsetPos = gameObject->transform.GetGlobalPosition() + m_positionOffset;
        transform[3] = glm::vec4(offsetPos, 1.0f);

        std::array<glm::vec3, 8> vertices = {
                glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z), glm::vec3(halfSize.x, -halfSize.y, -halfSize.z),
                glm::vec3(halfSize.x, halfSize.y, -halfSize.z),   glm::vec3(-halfSize.x, halfSize.y, -halfSize.z),
                glm::vec3(-halfSize.x, -halfSize.y, halfSize.z),  glm::vec3(halfSize.x, -halfSize.y, halfSize.z),
                glm::vec3(halfSize.x, halfSize.y, halfSize.z),    glm::vec3(-halfSize.x, halfSize.y, halfSize.z)};

        m_WorldMin = glm::vec3(std::numeric_limits<float>::max());
        m_WorldMax = glm::vec3(std::numeric_limits<float>::lowest());

        for (const auto &vertex: vertices) {
            glm::vec3 worldPos = glm::vec3(transform * glm::vec4(vertex, 1.0f));
            m_WorldMin = glm::min(m_WorldMin, worldPos);
            m_WorldMax = glm::max(m_WorldMax, worldPos);
        }
    } else if (m_shapeType == ShapeType::CAPSULE) {
        glm::vec3 pos = gameObject->transform.GetGlobalPosition() + m_positionOffset;
        float radius =
                m_capsuleRadius * (gameObject->transform.GetScale().x + gameObject->transform.GetScale().z) * 0.5f;
        float halfHeight = m_capsuleHeight * gameObject->transform.GetScale().y * 0.5f - radius;

        glm::quat rotation = gameObject->transform.GetQuatRotation();
        glm::vec3 axis = glm::normalize(rotation * glm::vec3(0.0f, 1.0f, 0.0f)) * halfHeight;

        glm::vec3 top = pos + axis;
        glm::vec3 bottom = pos - axis;

        m_WorldMin = glm::min(top, bottom) - glm::vec3(radius);
        m_WorldMax = glm::max(top, bottom) + glm::vec3(radius);
    } else {
        const float DEFAULT_SIZE = 0.1f;
        glm::vec3 pos = gameObject->transform.GetGlobalPosition();
        m_WorldMin = pos - glm::vec3(DEFAULT_SIZE);
        m_WorldMax = pos + glm::vec3(DEFAULT_SIZE);
    }

    m_aabbDirty = false;
}

void CollisionShape::GetWorldAABB(glm::vec3 &outMin, glm::vec3 &outMax) {
    UpdateAABB();
    outMin = m_WorldMin;
    outMax = m_WorldMax;
}

void CollisionShape::SetCollisionLayer(uint8_t layer) { m_collisionLayer = layer; }

void CollisionShape::SetCollisionMask(uint8_t mask) { m_collisionMask = mask; }

void CollisionShape::SetLayerBit(int bitIndex, bool enabled) {
    if (bitIndex < 0 || bitIndex >= 8)
        return;

    if (enabled) {
        m_collisionLayer |= (1 << bitIndex);
    } else {
        m_collisionLayer &= ~(1 << bitIndex);
    }
}

void CollisionShape::SetMaskBit(int bitIndex, bool enabled) {
    if (bitIndex < 0 || bitIndex >= 8)
        return;

    if (enabled) {
        m_collisionMask |= (1 << bitIndex);
    } else {
        m_collisionMask &= ~(1 << bitIndex);
    }
}

bool CollisionShape::GetLayerBit(int bitIndex) const {
    if (bitIndex < 0 || bitIndex >= 8)
        return false;
    return (m_collisionLayer & (1 << bitIndex)) != 0;
}

bool CollisionShape::GetMaskBit(int bitIndex) const {
    if (bitIndex < 0 || bitIndex >= 8)
        return false;
    return (m_collisionMask & (1 << bitIndex)) != 0;
}

bool CollisionShape::CanCollideWith(const CollisionShape *other) const {
    if (!other)
        return false;
    return ((m_collisionMask & other->m_collisionLayer) != 0) || ((other->m_collisionMask & m_collisionLayer) != 0);
}
