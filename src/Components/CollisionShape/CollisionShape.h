//
// Created by Hubert Klonowski on 19/03/2025.
//

#ifndef COLLISIONSHAPE_H
#define COLLISIONSHAPE_H

#include <glm/glm.hpp>
#include <vector>
#include "Component/Component.h"
#include "Shader/Shader.h"
#include "glad/glad.h"
#include "Signal/Signal.h"

enum class ShapeType { BOX, CAPSULE };

class CollisionShape : public Component {
public:
    CollisionShape(std::string SignalMessage_OnAreaEntered = "");
    ~CollisionShape() override;

    void Update(float deltaTime) override;

    void SetBoxSize(const glm::vec3 &size);
    void SetCapsuleParams(float radius, float height);
    void SetShapeType(ShapeType type);
    void SetShapeType(int type);

    void SetColor(const glm::vec3 &color);
    void SetVisible(bool visible);
    void SetWorldMin(glm::vec3 worldMin);
    void SetWorldMax(glm::vec3 worldMax);
    void SetIsCollisionArea(bool m_IsCollisionArea);
    void SetHasEnteredArea(bool m_HasEnteredInArea);

    ShapeType GetShapeType() const { return m_shapeType; }
    glm::vec3 GetBoxSize() const { return m_boxSize; }
    float GetCapsuleRadius() const { return m_capsuleRadius; }
    float GetCapsuleHeight() const { return m_capsuleHeight; }
    bool IsVisible() const { return m_isVisible; }
    glm::vec3 GetColor() const { return m_color; }
    glm::vec3 GetWorldMin() const { return m_WorldMin; }
    glm::vec3 GetWorldMax() const { return m_WorldMax; }
    bool GetIsCollisionArea() const { return m_IsCollisionArea; }
    bool GetHasEnteredArea() const { return m_HasEnteredInArea; }

    void SetCollisionLayer(uint8_t layer);
    void SetCollisionMask(uint8_t mask);
    void SetLayerBit(int bitIndex, bool enabled);
    void SetMaskBit(int bitIndex, bool enabled);
    bool GetLayerBit(int bitIndex) const;
    bool GetMaskBit(int bitIndex) const;
    uint8_t GetCollisionLayer() const { return m_collisionLayer; }
    uint8_t GetCollisionMask() const { return m_collisionMask; }

    bool CanCollideWith(const CollisionShape *other) const;

    void Render(Shader *shader) const;
    static void RenderColliders(Shader *shader);
    static std::vector<CollisionShape *> colliders;
    void GetWorldAABB(glm::vec3 &outMin, glm::vec3 &outMax);
    void UpdateAABB();
    void MarkAABBDirty();
    virtual bool IsHitbox() const { return false; }
    /// <summary>
    /// Message of signal onareaneterd
    /// </summary>
    std::string SignalMessage_OnAreaEntered;
    void SetPositionOffset(const glm::vec3 &offset) {
        m_positionOffset = offset;
        MarkAABBDirty();
    }
    glm::vec3 GetPositionOffset() const { return m_positionOffset; }
    std::vector<GameObject *> gameObjectsInArea;


private:
    ShapeType m_shapeType;
    glm::vec3 m_boxSize;
    float m_capsuleRadius;
    float m_capsuleHeight;
    glm::vec3 m_color;
    bool m_isVisible = true;
    glm::vec3 m_WorldMin;
    glm::vec3 m_WorldMax;
    bool m_aabbDirty = true;
    bool m_IsCollisionArea = false;
    bool m_HasEnteredInArea = false;
    glm::vec3 m_positionOffset = glm::vec3(0.0f, 0.0f, 0.0f);

    uint8_t m_collisionLayer = 1;
    uint8_t m_collisionMask = 1; 

};

#endif // COLLISIONSHAPE_H
