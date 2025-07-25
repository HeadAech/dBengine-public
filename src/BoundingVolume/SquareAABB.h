#ifndef SQUARE_AABB_H
#define SQUARE_AABB_H

#include "BoundingVolume.h"

struct SquareAABB : public BoundingVolume {
    glm::vec3 center{0.f, 0.f, 0.f};
    float extent{0.f};

    SquareAABB(const glm::vec3 &inCenter, float inExtent) : BoundingVolume{}, center{inCenter}, extent{inExtent} {}

    bool IsOnOrForwardPlane(const Plane &plane) const final {
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        const float r = extent * (std::abs(plane.normal.x) + std::abs(plane.normal.y) + std::abs(plane.normal.z));
        return -r <= plane.GetSignedDistanceToPlane(center);
    }

    bool IsOnFrustum(const Frustum &camFrustum, const Transform &transform) const final {
        // Get global scale thanks to our transform
        const glm::vec3 globalCenter{transform.GetModelMatrix() * glm::vec4(center, 1.f)};

        // Scaled orientation
        const glm::vec3 right = transform.GetRight() * extent;
        const glm::vec3 up = transform.GetUp() * extent;
        const glm::vec3 forward = transform.GetForward() * extent;

        const float newIi = std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, right)) +
                            std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, up)) +
                            std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, forward));

        const float newIj = std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, right)) +
                            std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, up)) +
                            std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, forward));

        const float newIk = std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, right)) +
                            std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, up)) +
                            std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, forward));

        const SquareAABB globalAABB(globalCenter, std::max(std::max(newIi, newIj), newIk));

        return (globalAABB.IsOnOrForwardPlane(camFrustum.leftFace) &&
                globalAABB.IsOnOrForwardPlane(camFrustum.rightFace) &&
                globalAABB.IsOnOrForwardPlane(camFrustum.topFace) &&
                globalAABB.IsOnOrForwardPlane(camFrustum.bottomFace) &&
                globalAABB.IsOnOrForwardPlane(camFrustum.nearFace) &&
                globalAABB.IsOnOrForwardPlane(camFrustum.farFace));
    };
};

#endif // !SQUARE_AABB_H
