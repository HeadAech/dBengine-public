//
// Created by Hubert Klonowski on 25/03/2025.
//

#ifndef DBPHYSICS_H
#define DBPHYSICS_H

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include "BoundingVolume/AABB.h"
#include "Components/CollisionShape/CollisionShape.h"
#include "Components/PhysicsBody/PhysicsBody.h"
#include "GameObject/GameObject.h"
#include "Octree/Octree.h"
#include "Components/Hitbox/Hitbox.h"

struct RaycastHit {
    bool hit = false;
    CollisionShape *collider = nullptr;
    glm::vec3 point = glm::vec3(0.0f);
    glm::vec3 normal = glm::vec3(0.0f);
    float distance = 0.0f;
    GameObject *gameObject = nullptr;
};

class dBphysics {
public:
    struct BoxTransform {
        glm::vec3 pos;
        glm::quat rot;
        glm::vec3 scale;
        glm::vec3 size;
    };

    struct CapsuleTransform {
        glm::vec3 pos;
        glm::quat rot;
        glm::vec3 scale;
        float radius;
        float height;
    };

    struct CapsuleSegment {
        glm::vec3 start;
        glm::vec3 end;
    };

    static dBphysics &GetInstance();
    void Initialize();
    void setInitialized(bool initialized);
    void Update(float deltaTime);
    void RemoveCollisionShape(CollisionShape *collider);
    void resolvePenetration(GameObject *a, GameObject *b, const glm::vec3 &normal, float depth);
    void applyCollisionImpulse(GameObject *a, GameObject *b, const glm::vec3 &normal, float depth);

    glm::vec3 closestPointOnLineSegment(const glm::vec3 &point, const glm::vec3 &lineStart, const glm::vec3 &lineEnd);
    void closestPointsOnTwoLines(const glm::vec3 &line1Start, const glm::vec3 &line1End, const glm::vec3 &line2Start,
                                 const glm::vec3 &line2End, glm::vec3 &point1, glm::vec3 &point2);
    glm::vec3 closestPointOnBox(const glm::vec3 &point, const glm::vec3 &boxCenter, const glm::vec3 &boxHalfSize,
                                const glm::quat &boxRotation);

    void SetGravity(const glm::vec3 &gravity) { m_gravity = gravity; }
    glm::vec3 GetGravity() const { return m_gravity; }

    void SetOctreeVisible(bool visible) { m_octree.SetVisible(visible); }
    bool IsOctreeVisible() const { return m_octree.IsVisible(); }
    void RenderOctree(Shader *shader) const { m_octree.Render(shader); }
    std::vector<Octree::OctreeDebugInfo> GetOctreeDebugInfo() const { return m_octree.GetDebugInfo(); }
    void SetOctreeBounds(const glm::vec3 &min, const glm::vec3 &max) { m_octree.SetBounds(min, max); }
    glm::vec3 GetOctreeMin() const { return m_octree.GetMin(); }
    glm::vec3 GetOctreeMax() const { return m_octree.GetMax(); }
    void ForceOctreeRebuild() { m_octreeNeedsRebuild = true; }

    bool RaycastBool(const glm::vec3 &origin, const glm::vec3 &direction, float maxDistance = FLT_MAX,
                     std::unordered_set<const CollisionShape *> ignoredColliders = {});
    RaycastHit Raycast(const glm::vec3 &origin, const glm::vec3 &direction, float maxDistance = FLT_MAX,
                       std::unordered_set<const CollisionShape *> ignoredColliders = {});

private:
    dBphysics();
    ~dBphysics();
    dBphysics(const dBphysics &) = delete;
    dBphysics &operator=(const dBphysics &) = delete;

    bool m_initialized = false;
    glm::vec3 m_gravity = glm::vec3(0.0f, -9.81f, 0.0f);
    std::vector<CollisionShape *> m_staticColliders;
    std::vector<CollisionShape *> m_dynamicColliders;
    std::vector<CollisionShape *> m_allColliders;
    Octree m_octree;

    bool m_octreeNeedsRebuild = true;
    bool m_octreeInitialized = false; 
    std::unordered_set<CollisionShape *> m_dirtyColliders;
    std::unordered_map<CollisionShape *, glm::vec3> m_lastKnownPositions;

    void registerColliders();
    void resetGroundState();
    void integrateObjects(float deltaTime);
    void processCollisionPair(CollisionShape *a, CollisionShape *b, int iteration, int totalIterations);
    bool processCollision(int iteration, int totalIterations);
    void resolveCollisions(int totalIterations);
    void updateGroundState(PhysicsBody *rbA, PhysicsBody *rbB, const glm::vec3 &normal);
    void applyFriction(PhysicsBody *rbA, PhysicsBody *rbB, const glm::vec3 &relVel, const glm::vec3 &normal,
                       float invMassSum, float normalImpulse);

    void handleDegenerateLines(const glm::vec3 &line1Start, const glm::vec3 &line1End, const glm::vec3 &line2Start,
                               const glm::vec3 &line2End, glm::vec3 &point1, glm::vec3 &point2, float a, float e,
                               float EPSILON);
    void handleParallelLines(const glm::vec3 &line1Start, const glm::vec3 &line1End, const glm::vec3 &line2Start,
                             const glm::vec3 &line2End, glm::vec3 &point1, glm::vec3 &point2, const glm::vec3 &d1,
                             const glm::vec3 &d2, float f, float e);

    bool checkCollision(CollisionShape *a, CollisionShape *b, glm::vec3 &normal, float &depth);
    bool checkBoxBox(CollisionShape *a, CollisionShape *b, glm::vec3 &normal, float &depth);
    bool checkCapsuleCapsule(CollisionShape *a, CollisionShape *b, glm::vec3 &normal, float &depth);
    bool checkBoxCapsule(CollisionShape *box, CollisionShape *capsule, glm::vec3 &normal, float &depth);

    BoxTransform getBoxTransform(CollisionShape *shape);
    void setupSeparatingAxes(glm::vec3 axes[15], const glm::mat3 &rotMatA, const glm::mat3 &rotMatB);
    bool findMinimumSeparation(glm::vec3 axes[15], const glm::vec3 &relativePos, const glm::mat3 &rotMatA,
                               const glm::mat3 &rotMatB, const glm::vec3 &sizeA, const glm::vec3 &sizeB,
                               glm::vec3 &normal, float &depth);
    float calculateAxisOverlap(const glm::vec3 &axis, const glm::vec3 &relativePos, const glm::mat3 &rotMatA,
                               const glm::mat3 &rotMatB, const glm::vec3 &sizeA, const glm::vec3 &sizeB);

    CapsuleTransform getCapsuleTransform(CollisionShape *shape);
    CapsuleSegment getCapsuleLineSegment(const glm::vec3 &pos, const glm::quat &rot, float height, float radius);
    glm::vec3 calculateCapsuleFallbackNormal(const glm::vec3 &posA, const glm::vec3 &posB);
    glm::vec3 calculateBoxCapsuleFallbackNormal(const glm::vec3 &capsulePoint, const glm::vec3 &boxPos,
                                                const glm::vec3 &boxSize, const glm::quat &boxRot);

    void initializeOctree();
    void updateOctree();
    void checkForDirtyColliders();
    void markColliderDirty(CollisionShape *collider);
    void cleanupRemovedColliders();
    bool processCollisionOctree(int iteration, int totalIterations);

    bool shouldCollide(CollisionShape *a, CollisionShape *b);

    bool raycastBox(const glm::vec3 &origin, const glm::vec3 &direction, CollisionShape *box, RaycastHit &hit);
    bool raycastCapsule(const glm::vec3 &origin, const glm::vec3 &direction, CollisionShape *capsule, RaycastHit &hit);
    bool raycastShape(const glm::vec3 &origin, const glm::vec3 &direction, CollisionShape *shape, RaycastHit &hit);
};

#endif // DBPHYSICS_H
