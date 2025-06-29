//
// Created by Hubert Klonowski on 25/03/2025.
//

#include "dBphysics.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "dBengine/EngineDebug/EngineDebug.h"
#include "dBrender/dBrender.h"

dBphysics &dBphysics::GetInstance() {
    static dBphysics instance;
    return instance;
}

dBphysics::dBphysics() : m_gravity(0.0f, -30.81f, 0.0f) {}

dBphysics::~dBphysics() {}

void dBphysics::Initialize() {
    m_initialized = true;
    m_staticColliders.clear();
    m_dynamicColliders.clear();
    registerColliders();

    initializeOctree();

    EngineDebug::GetInstance().PrintInfo("Physics system initialized with " +
                                         std::to_string(m_dynamicColliders.size()) + " dynamic and " +
                                         std::to_string(m_staticColliders.size()) + " static colliders");
}

void dBphysics::setInitialized(bool initialized) {
    this->m_initialized = initialized;
}

void dBphysics::registerColliders() {
    size_t oldSize = m_allColliders.size();
    m_staticColliders.clear();
    m_dynamicColliders.clear();
    m_allColliders.clear();

    for (auto collider: CollisionShape::colliders) {
        if (collider && collider->enabled && collider->gameObject && collider->gameObject->m_enabled) {
            collider->gameObject->ForceUpdateTransform();
            m_allColliders.push_back(collider);

            PhysicsBody *pb = collider->gameObject->GetComponent<PhysicsBody>();
            bool isStatic = !pb || pb->isStatic;
            isStatic = isStatic && !collider->GetIsCollisionArea();
            if (isStatic) {
                m_staticColliders.push_back(collider);
            } else {
                m_dynamicColliders.push_back(collider);
            }
        }
    }

    if (m_allColliders.size() != oldSize) {
        m_octreeNeedsRebuild = true;
    }
}

void dBphysics::resetGroundState() {
    for (CollisionShape *collider: m_dynamicColliders) {
        if (auto pb = collider->gameObject->GetComponent<PhysicsBody>()) {
            pb->isGrounded = false;
        }
    }
}

void dBphysics::integrateObjects(float deltaTime) {
    for (CollisionShape *collider: m_allColliders) {
        if (!collider || !collider->enabled || !collider->gameObject || !collider->gameObject->m_enabled)
            continue;

        PhysicsBody *pb = collider->gameObject->GetComponent<PhysicsBody>();
        if (pb && pb->enabled) {
            pb->Integrate(deltaTime);
        }
    }
}

void dBphysics::processCollisionPair(CollisionShape *a, CollisionShape *b, int iteration, int totalIterations) {
    glm::vec3 normal;
    float depth;

    if (checkCollision(a, b, normal, depth)) {
        bool aIsHitbox = a->IsHitbox();
        bool bIsHitbox = b->IsHitbox();

        if (aIsHitbox || bIsHitbox) {
            if (aIsHitbox && !bIsHitbox) {
                Hitbox *hitbox = static_cast<Hitbox *>(a);
                hitbox->OnHitTarget(b);
            } else if (bIsHitbox && !aIsHitbox) {
                Hitbox *hitbox = static_cast<Hitbox *>(b);
                hitbox->OnHitTarget(a);
            }
            return;
        }

            //AREAs are considered as dynamic, but are static in terms of moving [mostly it should be A instead of B because of it].
        if (a->GetIsCollisionArea() || b->GetIsCollisionArea()) {
            if (a->GetIsCollisionArea() && std::find(a->gameObjectsInArea.begin(), a->gameObjectsInArea.end(), b->gameObject) == a->gameObjectsInArea.end()) {
                Signals::Collision_OnAreaEnter.emit(b->SignalMessage_OnAreaEntered);
                a->gameObjectsInArea.push_back(b->gameObject);
                a->SetHasEnteredArea(true);
            } else if (b->GetIsCollisionArea() && std::find(b->gameObjectsInArea.begin(), b->gameObjectsInArea.end(), a->gameObject) == b->gameObjectsInArea.end()) {
                Signals::Collision_OnAreaEnter.emit(b->SignalMessage_OnAreaEntered);
                b->gameObjectsInArea.push_back(a->gameObject);
                b->SetHasEnteredArea(true);
            }
        }
        else {
            float iterationFactor = 1.0f / (float) totalIterations;
            resolvePenetration(a->gameObject, b->gameObject, normal, depth * iterationFactor);

            if (iteration == 0 && !a->GetIsCollisionArea() && !b->GetIsCollisionArea()) {
                applyCollisionImpulse(a->gameObject, b->gameObject, normal, depth);
            }
        }
    }
}

bool dBphysics::processCollision(int iteration, int totalIterations) {
    bool foundCollision = false;

    for (size_t i = 0; i < m_dynamicColliders.size(); i++) {
        for (size_t j = i + 1; j < m_dynamicColliders.size(); j++) {
            CollisionShape *a = m_dynamicColliders[i];
            CollisionShape *b = m_dynamicColliders[j];
            if (!shouldCollide(a, b)) {
                continue;
            }

            glm::vec3 normal;
            float depth;
            if (checkCollision(a, b, normal, depth)) {
                foundCollision = true;
                processCollisionPair(a, b, iteration, totalIterations);
            }
        }
    }

    for (CollisionShape *a: m_dynamicColliders) {
        for (CollisionShape *b: m_staticColliders) {
            if (!shouldCollide(a, b)) {
                continue;
            }
            glm::vec3 normal;
            float depth;
            if (checkCollision(a, b, normal, depth)) {
                foundCollision = true;
                processCollisionPair(a, b, iteration, totalIterations);
            }
        }
    }

    return foundCollision;
}

void dBphysics::resolveCollisions(int totalIterations) {
    for (int i = 0; i < totalIterations; i++) {
        if (!processCollisionOctree(i, totalIterations)) {
            break;
        }
    }
}

void dBphysics::Update(float deltaTime) {
    if (!m_initialized) {
        Initialize();
    }
    float maxDeltaTime = 1.0f / 60.0f; 
    float clampDeltaTime = glm::min(deltaTime, maxDeltaTime);

    if (!m_octreeInitialized) {
        initializeOctree();
    } else {
        checkForDirtyColliders();
        cleanupRemovedColliders();
        updateOctree();
    }

    registerColliders();
    resetGroundState();
    integrateObjects(clampDeltaTime); 
    resolveCollisions(2); 
}

void dBphysics::resolvePenetration(GameObject *a, GameObject *b, const glm::vec3 &normal, float depth) {
    PhysicsBody *pbA = a->GetComponent<PhysicsBody>();
    PhysicsBody *pbB = b->GetComponent<PhysicsBody>();

    if (!pbA || !pbB)
        return;

    float totalInvMass = pbA->invMass + pbB->invMass;
    if (totalInvMass <= 0.0001f)
        return;

    const float penetrationAllowance = 0.001f;
    float correctionDepth = glm::max(0.0f, depth - penetrationAllowance);
    if (correctionDepth <= 0.0f)
        return;

    glm::vec3 correction = normal * (correctionDepth / totalInvMass);

    if (!pbA->isStatic) {
        glm::vec3 posA = a->transform.GetLocalPosition();
        posA -= correction * pbA->invMass;
        a->transform.SetLocalPosition(posA);
    }

    if (!pbB->isStatic) {
        glm::vec3 posB = b->transform.GetLocalPosition();
        posB += correction * pbB->invMass;
        b->transform.SetLocalPosition(posB);
    }

    updateGroundState(pbA, pbB, normal);
}

void dBphysics::updateGroundState(PhysicsBody *pbA, PhysicsBody *pbB, const glm::vec3 &normal) {
    float dotUp = glm::dot(normal, glm::vec3(0.0f, 1.0f, 0.0f));
    float dotDown = glm::dot(normal, glm::vec3(0.0f, -1.0f, 0.0f));

    if (pbA && !pbA->isStatic && dotDown > 0.7f) {
        pbA->isGrounded = true;
    }
    if (pbB && !pbB->isStatic && dotUp > 0.7f) {
        pbB->isGrounded = true;
    }
}

void dBphysics::applyCollisionImpulse(GameObject *a, GameObject *b, const glm::vec3 &normal, float depth) {
    PhysicsBody *pbA = a->GetComponent<PhysicsBody>();
    PhysicsBody *pbB = b->GetComponent<PhysicsBody>();

    if (!pbA || !pbB)
        return;

    glm::vec3 relVel = pbB->velocity - pbA->velocity;
    float velAlongNormal = glm::dot(relVel, normal);

    if (velAlongNormal > 0.0f || std::abs(velAlongNormal) < 0.05f)
        return;

    float restitution = glm::min(pbA->restitution, pbB->restitution);
    float invMassSum = pbA->invMass + pbB->invMass;
    if (invMassSum <= 0.0001f)
        return;

    float j = -(1.0f + restitution) * velAlongNormal / invMassSum;
    glm::vec3 impulse = j * normal;

    if (!pbA->isStatic)
        pbA->velocity -= impulse * pbA->invMass;
    if (!pbB->isStatic)
        pbB->velocity += impulse * pbB->invMass;

    applyFriction(pbA, pbB, relVel, normal, invMassSum, j);
}

void dBphysics::applyFriction(PhysicsBody *pbA, PhysicsBody *pbB, const glm::vec3 &relVel, const glm::vec3 &normal,
                              float invMassSum, float normalImpulse) {
    const float frictionCoefficient = 2.5f;
    float velAlongNormal = glm::dot(relVel, normal);
    glm::vec3 tangent = relVel - (normal * velAlongNormal);
    float tangentLength = glm::length(tangent);

    if (tangentLength <= 0.0001f)
        return;

    tangent = tangent / tangentLength;
    float jt = -glm::dot(relVel, tangent) / invMassSum;
    float fricImpulseLimit = normalImpulse * frictionCoefficient;
    jt = glm::clamp(jt, -fricImpulseLimit, fricImpulseLimit);

    glm::vec3 frictionImpulse = jt * tangent;

    if (!pbA->isStatic)
        pbA->velocity -= frictionImpulse * pbA->invMass;
    if (!pbB->isStatic)
        pbB->velocity += frictionImpulse * pbB->invMass;
}

glm::vec3 dBphysics::closestPointOnLineSegment(const glm::vec3 &point, const glm::vec3 &lineStart,
                                               const glm::vec3 &lineEnd) {
    glm::vec3 lineDirection = lineEnd - lineStart;
    float lineLength = glm::length(lineDirection);

    if (lineLength < 0.0001f)
        return lineStart;

    glm::vec3 lineDir = lineDirection / lineLength;
    float t = glm::clamp(glm::dot(point - lineStart, lineDir), 0.0f, lineLength);
    return lineStart + lineDir * t;
}

void dBphysics::closestPointsOnTwoLines(const glm::vec3 &line1Start, const glm::vec3 &line1End,
                                        const glm::vec3 &line2Start, const glm::vec3 &line2End, glm::vec3 &point1,
                                        glm::vec3 &point2) {
    const float EPSILON = 0.0001f;
    glm::vec3 d1 = line1End - line1Start;
    glm::vec3 d2 = line2End - line2Start;
    glm::vec3 r = line1Start - line2Start;

    float a = glm::dot(d1, d1);
    float e = glm::dot(d2, d2);
    float f = glm::dot(d2, r);

    if (a < EPSILON || e < EPSILON) {
        handleDegenerateLines(line1Start, line1End, line2Start, line2End, point1, point2, a, e, EPSILON);
        return;
    }

    float b = glm::dot(d1, d2);
    float c = glm::dot(d1, r);
    float det = a * e - b * b;

    if (det > EPSILON) {
        float s = glm::clamp((b * f - c * e) / det, 0.0f, 1.0f);
        float t = glm::clamp((a * f - b * c) / det, 0.0f, 1.0f);
        point1 = line1Start + d1 * s;
        point2 = line2Start + d2 * t;
    } else {
        handleParallelLines(line1Start, line1End, line2Start, line2End, point1, point2, d1, d2, f, e);
    }
}

void dBphysics::handleDegenerateLines(const glm::vec3 &line1Start, const glm::vec3 &line1End,
                                      const glm::vec3 &line2Start, const glm::vec3 &line2End, glm::vec3 &point1,
                                      glm::vec3 &point2, float a, float e, float EPSILON) {
    if (a < EPSILON && e < EPSILON) {
        point1 = line1Start;
        point2 = line2Start;
    } else if (a < EPSILON) {
        point1 = line1Start;
        point2 = closestPointOnLineSegment(line1Start, line2Start, line2End);
    } else {
        point2 = line2Start;
        point1 = closestPointOnLineSegment(line2Start, line1Start, line1End);
    }
}

void dBphysics::handleParallelLines(const glm::vec3 &line1Start, const glm::vec3 &line1End, const glm::vec3 &line2Start,
                                    const glm::vec3 &line2End, glm::vec3 &point1, glm::vec3 &point2,
                                    const glm::vec3 &d1, const glm::vec3 &d2, float f, float e) {
    float s = 0.0f;
    float t = glm::clamp(f / e, 0.0f, 1.0f);

    float minDist = glm::length2(line1Start - (line2Start + d2 * t));
    point1 = line1Start;
    point2 = line2Start + d2 * t;

    float tempDist = glm::length2(line1End - (line2Start + d2 * t));
    if (tempDist < minDist) {
        minDist = tempDist;
        s = 1.0f;
        point1 = line1End;
    }

    tempDist = glm::length2(line1Start - line2End);
    if (tempDist < minDist) {
        minDist = tempDist;
        s = 0.0f;
        t = 1.0f;
        point1 = line1Start;
        point2 = line2End;
    }

    tempDist = glm::length2(line1End - line2End);
    if (tempDist < minDist) {
        s = 1.0f;
        t = 1.0f;
        point1 = line1End;
        point2 = line2End;
    }
}

glm::vec3 dBphysics::closestPointOnBox(const glm::vec3 &point, const glm::vec3 &boxCenter, const glm::vec3 &boxHalfSize,
                                       const glm::quat &boxRotation) {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), boxCenter) * glm::mat4_cast(boxRotation);
    glm::mat4 invTransform = glm::inverse(transform);
    glm::vec3 localPoint = glm::vec3(invTransform * glm::vec4(point, 1.0f));

    glm::vec3 clampedLocalPoint;
    for (int i = 0; i < 3; ++i) {
        clampedLocalPoint[i] = glm::clamp(localPoint[i], -boxHalfSize[i], boxHalfSize[i]);
    }

    return glm::vec3(transform * glm::vec4(clampedLocalPoint, 1.0f));
}

bool dBphysics::checkCollision(CollisionShape *a, CollisionShape *b, glm::vec3 &normal, float &depth) {
    ShapeType typeA = a->GetShapeType();
    ShapeType typeB = b->GetShapeType();

    if (typeA == ShapeType::BOX && typeB == ShapeType::BOX) {
        return checkBoxBox(a, b, normal, depth);
    }
    if (typeA == ShapeType::CAPSULE && typeB == ShapeType::CAPSULE) {
        return checkCapsuleCapsule(a, b, normal, depth);
    }
    if (typeA == ShapeType::BOX && typeB == ShapeType::CAPSULE) {
        return checkBoxCapsule(a, b, normal, depth);
    }
    if (typeA == ShapeType::CAPSULE && typeB == ShapeType::BOX) {
        bool result = checkBoxCapsule(b, a, normal, depth);
        normal = -normal;
        return result;
    }
    return false;
}

bool dBphysics::checkBoxBox(CollisionShape *a, CollisionShape *b, glm::vec3 &normal, float &depth) {
    if (!a || !b || !a->gameObject || !b->gameObject)
        return false;

    BoxTransform transformA = getBoxTransform(a);
    BoxTransform transformB = getBoxTransform(b);

    glm::vec3 relativePos = transformB.pos - transformA.pos;
    glm::mat3 rotMatA = glm::mat3_cast(transformA.rot);
    glm::mat3 rotMatB = glm::mat3_cast(transformB.rot);

    glm::vec3 axes[15];
    setupSeparatingAxes(axes, rotMatA, rotMatB);

    return findMinimumSeparation(axes, relativePos, rotMatA, rotMatB, transformA.size, transformB.size, normal, depth);
}

dBphysics::BoxTransform dBphysics::getBoxTransform(CollisionShape *shape) {
    BoxTransform transform;
    transform.pos = shape->gameObject->transform.GetGlobalPosition() + shape->GetPositionOffset();
    transform.rot = shape->gameObject->transform.GetQuatRotation();
    transform.scale = shape->gameObject->transform.GetScale();
    transform.size = shape->GetBoxSize() * transform.scale * 0.5f;
    return transform;
}

void dBphysics::setupSeparatingAxes(glm::vec3 axes[15], const glm::mat3 &rotMatA, const glm::mat3 &rotMatB) {
    for (int i = 0; i < 3; i++) {
        axes[i] = rotMatA[i];
        axes[3 + i] = rotMatB[i];
    }

    int axisIndex = 6;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            glm::vec3 cross = glm::cross(rotMatA[i], rotMatB[j]);
            axes[axisIndex++] = (glm::length2(cross) > 0.0001f) ? glm::normalize(cross) : glm::vec3(1, 0, 0);
        }
    }
}

bool dBphysics::findMinimumSeparation(glm::vec3 axes[15], const glm::vec3 &relativePos, const glm::mat3 &rotMatA,
                                      const glm::mat3 &rotMatB, const glm::vec3 &sizeA, const glm::vec3 &sizeB,
                                      glm::vec3 &normal, float &depth) {
    float minOverlap = FLT_MAX;
    glm::vec3 bestAxis;

    for (int i = 0; i < 15; i++) {
        glm::vec3 axis = axes[i];
        if (glm::length2(axis) < 0.0001f)
            continue;

        float overlap = calculateAxisOverlap(axis, relativePos, rotMatA, rotMatB, sizeA, sizeB);
        if (overlap <= 0.0f)
            return false;

        if (overlap < minOverlap) {
            minOverlap = overlap;
            bestAxis = axis;
            if (glm::dot(relativePos, axis) < 0.0f) {
                bestAxis = -bestAxis;
            }
        }
    }

    normal = glm::normalize(bestAxis);
    depth = minOverlap;
    return true;
}

float dBphysics::calculateAxisOverlap(const glm::vec3 &axis, const glm::vec3 &relativePos, const glm::mat3 &rotMatA,
                                      const glm::mat3 &rotMatB, const glm::vec3 &sizeA, const glm::vec3 &sizeB) {
    float relativeProjection = glm::dot(relativePos, axis);

    float projectionA = 0.0f;
    float projectionB = 0.0f;

    for (int j = 0; j < 3; j++) {
        projectionA += glm::abs(glm::dot(rotMatA[j], axis)) * sizeA[j];
        projectionB += glm::abs(glm::dot(rotMatB[j], axis)) * sizeB[j];
    }

    return projectionA + projectionB - glm::abs(relativeProjection);
}

bool dBphysics::checkCapsuleCapsule(CollisionShape *a, CollisionShape *b, glm::vec3 &normal, float &depth) {
    if (!a || !b || !a->gameObject || !b->gameObject)
        return false;

    CapsuleTransform transformA = getCapsuleTransform(a);
    CapsuleTransform transformB = getCapsuleTransform(b);

    CapsuleSegment segmentA =
            getCapsuleLineSegment(transformA.pos, transformA.rot, transformA.height, transformA.radius);
    CapsuleSegment segmentB =
            getCapsuleLineSegment(transformB.pos, transformB.rot, transformB.height, transformB.radius);

    glm::vec3 closestA, closestB;
    closestPointsOnTwoLines(segmentA.start, segmentA.end, segmentB.start, segmentB.end, closestA, closestB);

    glm::vec3 separation = closestB - closestA;
    float distance = glm::length(separation);
    float totalRadius = transformA.radius + transformB.radius;

    if (distance >= totalRadius)
        return false;

    normal = (distance > 0.0001f) ? separation / distance
                                  : calculateCapsuleFallbackNormal(transformA.pos, transformB.pos);
    depth = totalRadius - distance;
    return true;
}

dBphysics::CapsuleTransform dBphysics::getCapsuleTransform(CollisionShape *shape) {
    CapsuleTransform transform;
    transform.pos = shape->gameObject->transform.GetGlobalPosition() + shape->GetPositionOffset();
    transform.rot = shape->gameObject->transform.GetQuatRotation();
    transform.scale = shape->gameObject->transform.GetScale();
    transform.radius = shape->GetCapsuleRadius() * (transform.scale.x + transform.scale.z) * 0.5f;
    transform.height = shape->GetCapsuleHeight() * transform.scale.y;
    return transform;
}

dBphysics::CapsuleSegment dBphysics::getCapsuleLineSegment(const glm::vec3 &pos, const glm::quat &rot, float height,
                                                           float radius) {
    glm::vec3 up = rot * glm::vec3(0, 1, 0);
    float halfHeight = (height * 0.5f) - radius;
    CapsuleSegment segment;
    segment.start = pos - up * halfHeight;
    segment.end = pos + up * halfHeight;
    return segment;
}

glm::vec3 dBphysics::calculateCapsuleFallbackNormal(const glm::vec3 &posA, const glm::vec3 &posB) {
    glm::vec3 relativePos = posB - posA;
    return (glm::length2(relativePos) > 0.0001f) ? glm::normalize(relativePos) : glm::vec3(0, 1, 0);
}

bool dBphysics::checkBoxCapsule(CollisionShape *box, CollisionShape *capsule, glm::vec3 &normal, float &depth) {
    if (!box || !capsule || !box->gameObject || !capsule->gameObject)
        return false;

    BoxTransform boxTransform = getBoxTransform(box);
    CapsuleTransform capsuleTransform = getCapsuleTransform(capsule);

    CapsuleSegment capsuleSegment = getCapsuleLineSegment(capsuleTransform.pos, capsuleTransform.rot,
                                                          capsuleTransform.height, capsuleTransform.radius);

    glm::vec3 closestOnCapsule = closestPointOnLineSegment(boxTransform.pos, capsuleSegment.start, capsuleSegment.end);
    glm::vec3 closestOnBox = closestPointOnBox(closestOnCapsule, boxTransform.pos, boxTransform.size, boxTransform.rot);

    glm::vec3 separation = closestOnCapsule - closestOnBox;
    float distance = glm::length(separation);

    if (distance >= capsuleTransform.radius)
        return false;

    normal = (distance > 0.0001f) ? separation / distance
                                  : calculateBoxCapsuleFallbackNormal(closestOnCapsule, boxTransform.pos,
                                                                      boxTransform.size, boxTransform.rot);
    depth = capsuleTransform.radius - distance;
    return true;
}

glm::vec3 dBphysics::calculateBoxCapsuleFallbackNormal(const glm::vec3 &capsulePoint, const glm::vec3 &boxPos,
                                                       const glm::vec3 &boxSize, const glm::quat &boxRot) {
    glm::mat4 boxTransform = glm::translate(glm::mat4(1.0f), boxPos) * glm::mat4_cast(boxRot);
    glm::mat4 invBoxTransform = glm::inverse(boxTransform);
    glm::vec3 localPoint = glm::vec3(invBoxTransform * glm::vec4(capsulePoint, 1.0f));

    glm::vec3 distances = boxSize - glm::abs(localPoint);

    if (distances.x <= distances.y && distances.x <= distances.z) {
        return boxRot * glm::vec3(localPoint.x > 0 ? 1.0f : -1.0f, 0, 0);
    } else if (distances.y <= distances.z) {
        return boxRot * glm::vec3(0, localPoint.y > 0 ? 1.0f : -1.0f, 0);
    } else {
        return boxRot * glm::vec3(0, 0, localPoint.z > 0 ? 1.0f : -1.0f);
    }
}

void dBphysics::initializeOctree() {
    m_octree.Build(m_allColliders);
    m_octreeInitialized = true;
    m_octreeNeedsRebuild = false;
    m_dirtyColliders.clear();

    m_lastKnownPositions.clear();
    for (auto collider: m_allColliders) {
        if (collider && collider->gameObject) {
            m_lastKnownPositions[collider] = collider->gameObject->transform.GetGlobalPosition();
        }
    }

    EngineDebug::GetInstance().PrintInfo("Octree initialized with " + std::to_string(m_allColliders.size()) +
                                         " colliders");
}

void dBphysics::checkForDirtyColliders() {
    for (auto collider: m_allColliders) {
        if (!collider || !collider->gameObject)
            continue;

        Transform &transform = collider->gameObject->transform;
        if (transform.IsDirty()) {
            collider->MarkAABBDirty();
            markColliderDirty(collider);
        } else {
            glm::vec3 currentPos = transform.GetGlobalPosition();
            auto lastPosIt = m_lastKnownPositions.find(collider);

            if (lastPosIt != m_lastKnownPositions.end() && glm::length(currentPos - lastPosIt->second) > 0.01f) {
                collider->MarkAABBDirty();
                markColliderDirty(collider);
            }
        }
    }
}

void dBphysics::markColliderDirty(CollisionShape *collider) {
    if (!collider || !collider->gameObject)
        return;

    m_dirtyColliders.insert(collider);
    m_lastKnownPositions[collider] = collider->gameObject->transform.GetGlobalPosition();
}

void dBphysics::updateOctree() {
    // if too many dirty rebild octree
    if (m_octreeNeedsRebuild || m_dirtyColliders.size() > m_allColliders.size() * 0.5f) {
        m_octree.Build(m_allColliders);
        m_dirtyColliders.clear();
        m_octreeNeedsRebuild = false;
        return;
    }

    // update only dirty
    if (!m_dirtyColliders.empty()) {
        for (auto collider: m_dirtyColliders) {
            if (collider && collider->gameObject) {
                m_octree.Update(collider);
            }
        }

        //EngineDebug::GetInstance().PrintInfo("Updated " + std::to_string(m_dirtyColliders.size()) +
        //                                     " colliders in octree");
        m_dirtyColliders.clear();
    }
}

void dBphysics::cleanupRemovedColliders() {
    auto it = m_lastKnownPositions.begin();
    while (it != m_lastKnownPositions.end()) {
        CollisionShape *collider = it->first;

        bool found = std::find(m_allColliders.begin(), m_allColliders.end(), collider) != m_allColliders.end();

        if (!found || !collider || !collider->gameObject || !collider->enabled) {
            if (found) {
                m_octree.Remove(collider);
            }
            m_dirtyColliders.erase(collider);
            it = m_lastKnownPositions.erase(it);
        } else {
            ++it;
        }
    }
}

bool dBphysics::processCollisionOctree(int iteration, int totalIterations) {
    bool foundCollision = false;

    for (CollisionShape *a: m_dynamicColliders) {
        if (!a || !a->enabled || !a->gameObject || !a->gameObject->m_enabled)
            continue;

        bool areaExitedFlag = false;

        std::vector<CollisionShape *> potentialColliders;
        m_octree.GetPotentialColliders(a, potentialColliders);

        for (CollisionShape *b: potentialColliders) {
            if (!b || !b->enabled || !b->gameObject || !b->gameObject->m_enabled)
                continue;
            if (a == b)
                continue; 

            PhysicsBody *pbA = a->gameObject->GetComponent<PhysicsBody>();
            PhysicsBody *pbB = b->gameObject->GetComponent<PhysicsBody>();
            bool aIsDynamic = pbA && !pbA->isStatic;
            bool bIsDynamic = pbB && !pbB->isStatic;

            if (aIsDynamic && bIsDynamic && a > b) {
                continue;
            }

            if (!shouldCollide(a, b)) {
                continue;
            }

            glm::vec3 normal;
            float depth;
            if (checkCollision(a, b, normal, depth)) {
                foundCollision = true;
                processCollisionPair(a, b, iteration, totalIterations);
                areaExitedFlag = true;
            } else if (a->GetIsCollisionArea() && a->gameObjectsInArea.size() > 0) {
                a->gameObjectsInArea.erase(
                        std::remove(a->gameObjectsInArea.begin(), a->gameObjectsInArea.end(), b->gameObject),
                        a->gameObjectsInArea.end());
            }
        }
        if (a->GetIsCollisionArea() && a->GetHasEnteredArea() && !areaExitedFlag) {
            a->SetHasEnteredArea(false);
        }
    }

    return foundCollision;
}

bool dBphysics::RaycastBool(const glm::vec3 &origin, const glm::vec3 &direction, float maxDistance,
                            std::unordered_set<const CollisionShape *> ignoredColliders) {
    RaycastHit hit = Raycast(origin, direction, maxDistance, ignoredColliders);
    return hit.hit;
}

RaycastHit dBphysics::Raycast(const glm::vec3 &origin, const glm::vec3 &direction, float maxDistance,
                              std::unordered_set<const CollisionShape *> ignoredColliders) {
    if (!m_initialized) {
        Initialize();
    }

    RaycastHit hit;

    glm::vec3 normalizedDir = glm::normalize(direction);
    float closestDistance = maxDistance;
    bool foundHit = false;

    for (auto *collider: m_allColliders) {
        if (!collider || !collider->enabled || !collider->gameObject || !collider->gameObject->m_enabled) {
            continue;
        }

        bool shouldIgnore = false;
        if (!ignoredColliders.empty()) {
            for (const CollisionShape *c: ignoredColliders) {
                if (c->gameObject == collider->gameObject) {
                    shouldIgnore = true;
                    break;
                }
            }
        }

        if (shouldIgnore) {
            continue;
        }

        RaycastHit tempHit;
        if (raycastShape(origin, normalizedDir, collider, tempHit)) {
            if (tempHit.distance < closestDistance) {
                hit = tempHit;
                closestDistance = tempHit.distance;
                foundHit = true;
            }
        }
    }

    return hit;
}

bool dBphysics::raycastShape(const glm::vec3 &origin, const glm::vec3 &direction, CollisionShape *shape,
                             RaycastHit &hit) {
    switch (shape->GetShapeType()) {
        case ShapeType::BOX:
            return raycastBox(origin, direction, shape, hit);
        case ShapeType::CAPSULE:
            return raycastCapsule(origin, direction, shape, hit);
        default:
            return false;
    }
}

bool dBphysics::raycastBox(const glm::vec3 &origin, const glm::vec3 &direction, CollisionShape *box, RaycastHit &hit) {
    glm::vec3 boxPos = box->gameObject->transform.GetLocalPosition();
    glm::vec3 boxScale = box->gameObject->transform.GetScale();
    glm::vec3 boxSize = box->GetBoxSize() * boxScale;
    glm::quat boxRot = box->gameObject->transform.GetQuatRotation();

    // Transform ray to box local space
    glm::mat4 worldToBox = glm::inverse(glm::translate(glm::mat4(1.0f), boxPos) * glm::mat4_cast(boxRot));
    glm::vec3 localOrigin = glm::vec3(worldToBox * glm::vec4(origin, 1.0f));
    glm::vec3 localDir = glm::vec3(worldToBox * glm::vec4(direction, 0.0f));

    glm::vec3 boxMin = -boxSize * 0.5f;
    glm::vec3 boxMax = boxSize * 0.5f;

    // Ray-AABB intersection
    glm::vec3 invDir = 1.0f / localDir;
    glm::vec3 t1 = (boxMin - localOrigin) * invDir;
    glm::vec3 t2 = (boxMax - localOrigin) * invDir;

    glm::vec3 tMin = glm::min(t1, t2);
    glm::vec3 tMax = glm::max(t1, t2);

    float tNear = glm::max(glm::max(tMin.x, tMin.y), tMin.z);
    float tFar = glm::min(glm::min(tMax.x, tMax.y), tMax.z);

    if (tNear > tFar || tFar < 0.0f) {
        return false; // No intersection
    }

    float t = tNear >= 0.0f ? tNear : tFar;
    if (t < 0.0f) {
        return false;
    }

    // Calculate hit point and normal in local space
    glm::vec3 localHitPoint = localOrigin + localDir * t;
    glm::vec3 localNormal = glm::vec3(0.0f);

    // Determine which face was hit
    const float EPSILON = 0.001f;
    if (std::abs(localHitPoint.x - boxMin.x) < EPSILON)
        localNormal = glm::vec3(-1, 0, 0);
    else if (std::abs(localHitPoint.x - boxMax.x) < EPSILON)
        localNormal = glm::vec3(1, 0, 0);
    else if (std::abs(localHitPoint.y - boxMin.y) < EPSILON)
        localNormal = glm::vec3(0, -1, 0);
    else if (std::abs(localHitPoint.y - boxMax.y) < EPSILON)
        localNormal = glm::vec3(0, 1, 0);
    else if (std::abs(localHitPoint.z - boxMin.z) < EPSILON)
        localNormal = glm::vec3(0, 0, -1);
    else if (std::abs(localHitPoint.z - boxMax.z) < EPSILON)
        localNormal = glm::vec3(0, 0, 1);

    // Transform back to world space
    glm::mat4 boxToWorld = glm::translate(glm::mat4(1.0f), boxPos) * glm::mat4_cast(boxRot);

    hit.hit = true;
    hit.collider = box;
    hit.gameObject = box->gameObject;
    hit.point = glm::vec3(boxToWorld * glm::vec4(localHitPoint, 1.0f));
    hit.normal = glm::normalize(glm::vec3(boxToWorld * glm::vec4(localNormal, 0.0f)));
    hit.distance = glm::length(hit.point - origin);

    return true;
}

bool dBphysics::raycastCapsule(const glm::vec3 &origin, const glm::vec3 &direction, CollisionShape *capsule,
                               RaycastHit &hit) {
    glm::vec3 capsulePos = capsule->gameObject->transform.GetLocalPosition();
    glm::vec3 capsuleScale = capsule->gameObject->transform.GetScale();
    float capsuleRadius = capsule->GetCapsuleRadius() * (capsuleScale.x + capsuleScale.z) * 0.5f;
    float capsuleHalfHeight = capsule->GetCapsuleHeight() * capsuleScale.y * 0.5f;
    glm::quat capsuleRot = capsule->gameObject->transform.GetQuatRotation();

    glm::vec3 capsuleUp = glm::normalize(capsuleRot * glm::vec3(0.0f, 1.0f, 0.0f));

    // Capsule line segment endpoints
    glm::vec3 capsuleTop = capsulePos + capsuleUp * (capsuleHalfHeight - capsuleRadius);
    glm::vec3 capsuleBottom = capsulePos - capsuleUp * (capsuleHalfHeight - capsuleRadius);

    // Ray-capsule intersection
    glm::vec3 rayToBottom = capsuleBottom - origin;
    glm::vec3 capsuleAxis = capsuleTop - capsuleBottom;

    float rayDotAxis = glm::dot(direction, capsuleAxis);
    float rayDotToBottom = glm::dot(direction, rayToBottom);
    float axisDotToBottom = glm::dot(capsuleAxis, rayToBottom);
    float axisLengthSq = glm::dot(capsuleAxis, capsuleAxis);
    float rayLengthSq = glm::dot(direction, direction);

    float a = axisLengthSq * rayLengthSq - rayDotAxis * rayDotAxis;
    float b = 2.0f * (axisDotToBottom * rayDotAxis - rayDotToBottom * axisLengthSq);
    float c = axisLengthSq * (glm::dot(rayToBottom, rayToBottom) - capsuleRadius * capsuleRadius) -
              axisDotToBottom * axisDotToBottom;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0.0f) {
        return false; // No intersection
    }

    float sqrt_discriminant = std::sqrt(discriminant);
    float t1 = (-b - sqrt_discriminant) / (2.0f * a);
    float t2 = (-b + sqrt_discriminant) / (2.0f * a);

    float t = (t1 >= 0.0f) ? t1 : t2;
    if (t < 0.0f) {
        return false;
    }

    // Check if intersection point is within capsule height
    glm::vec3 hitPoint = origin + direction * t;
    glm::vec3 capsuleToHit = hitPoint - capsuleBottom;
    float projectionOnAxis = glm::dot(capsuleToHit, capsuleAxis) / axisLengthSq;

    if (projectionOnAxis < 0.0f || projectionOnAxis > 1.0f) {
        // Hit is outside the cylindrical part, check sphere caps
        glm::vec3 sphereCenter = (projectionOnAxis < 0.0f) ? capsuleBottom : capsuleTop;

        glm::vec3 toSphere = sphereCenter - origin;
        float sphereProj = glm::dot(toSphere, direction);

        if (sphereProj < 0.0f) {
            return false;
        }

        glm::vec3 closestPoint = origin + direction * sphereProj;
        float distToSphere = glm::length(closestPoint - sphereCenter);

        if (distToSphere > capsuleRadius) {
            return false;
        }

        float sphereIntersectDist = std::sqrt(capsuleRadius * capsuleRadius - distToSphere * distToSphere);
        t = sphereProj - sphereIntersectDist;

        if (t < 0.0f) {
            return false;
        }

        hitPoint = origin + direction * t;
        hit.normal = glm::normalize(hitPoint - sphereCenter);
    } else {
        // Hit is on cylindrical part
        glm::vec3 closestOnAxis = capsuleBottom + capsuleAxis * projectionOnAxis;
        hit.normal = glm::normalize(hitPoint - closestOnAxis);
    }

    hit.hit = true;
    hit.collider = capsule;
    hit.gameObject = capsule->gameObject;
    hit.point = hitPoint;
    hit.distance = glm::length(hitPoint - origin);

    return true;
}

bool dBphysics::shouldCollide(CollisionShape *a, CollisionShape *b) {
    if (!a || !b)
        return false;

    if (!a->CanCollideWith(b)) {
        return false;
    }

    return true;
}

void dBphysics::RemoveCollisionShape(CollisionShape *collider) {
    if (!collider)
        return;

    for (auto it = m_staticColliders.begin(); it != m_staticColliders.end(); ++it) {
        if (*it == collider) {
            m_staticColliders.erase(it);
            break;
        }
    }

    for (auto it = m_dynamicColliders.begin(); it != m_dynamicColliders.end(); ++it) {
        if (*it == collider) {
            m_dynamicColliders.erase(it);
            break;
        }
    }

    for (auto it = m_allColliders.begin(); it != m_allColliders.end(); ++it) {
        if (*it == collider) {
            m_allColliders.erase(it);
            break;
        }
    }

    m_dirtyColliders.erase(collider);
    m_lastKnownPositions.erase(collider);

    if (m_octreeInitialized) {
        m_octree.Remove(collider);
    }
}
