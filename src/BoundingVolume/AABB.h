#ifndef AABB_H
#define AABB_H

#include "BoundingVolume.h"
#include <array>
#include <Components/MeshInstance/Mesh.h>

struct AABB : public BoundingVolume {
    glm::vec3 center{0.f, 0.f, 0.f};
    glm::vec3 extents{0.f, 0.f, 0.f};

    glm::vec3 min;
    glm::vec3 max;

    AABB(const glm::vec3 &min, const glm::vec3 &max) :
        BoundingVolume{}, center{(max + min) * 0.5f}, extents{max.x - center.x, max.y - center.y, max.z - center.z} {
        this->min = min;
        this->max = max;
    }

    AABB(const glm::vec3 &inCenter, float iI, float iJ, float iK) :
        BoundingVolume{}, center{inCenter}, extents{iI, iJ, iK} {}

    std::array<glm::vec3, 8> getVertice() const {
        std::array<glm::vec3, 8> vertice;
        vertice[0] = {center.x - extents.x, center.y - extents.y, center.z - extents.z};
        vertice[1] = {center.x + extents.x, center.y - extents.y, center.z - extents.z};
        vertice[2] = {center.x - extents.x, center.y + extents.y, center.z - extents.z};
        vertice[3] = {center.x + extents.x, center.y + extents.y, center.z - extents.z};
        vertice[4] = {center.x - extents.x, center.y - extents.y, center.z + extents.z};
        vertice[5] = {center.x + extents.x, center.y - extents.y, center.z + extents.z};
        vertice[6] = {center.x - extents.x, center.y + extents.y, center.z + extents.z};
        vertice[7] = {center.x + extents.x, center.y + extents.y, center.z + extents.z};
        return vertice;
    }

    bool IsOnOrForwardPlane(const Plane &plane) const final {
        // Compute the projection interval radius of b onto L(t) = b.c + t * p.n
        const float r = extents.x * std::abs(plane.normal.x) + extents.y * std::abs(plane.normal.y) +
                        extents.z * std::abs(plane.normal.z);

        return -r <= plane.GetSignedDistanceToPlane(center);
    }

    bool IsOnFrustum(const Frustum &camFrustum, const Transform &transform) const final {
        // Get global scale thanks to our transform
        const glm::vec3 globalCenter{transform.GetModelMatrix() * glm::vec4(center, 1.f)};

        // Scaled orientation
        const glm::vec3 right = transform.GetRight() * extents.x;
        const glm::vec3 up = transform.GetUp() * extents.y;
        const glm::vec3 forward = transform.GetForward() * extents.z;

        const float newIi = std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, right)) +
                            std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, up)) +
                            std::abs(glm::dot(glm::vec3{1.f, 0.f, 0.f}, forward));

        const float newIj = std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, right)) +
                            std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, up)) +
                            std::abs(glm::dot(glm::vec3{0.f, 1.f, 0.f}, forward));

        const float newIk = std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, right)) +
                            std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, up)) +
                            std::abs(glm::dot(glm::vec3{0.f, 0.f, 1.f}, forward));

        const AABB globalAABB(globalCenter, newIi, newIj, newIk);

        return (globalAABB.IsOnOrForwardPlane(camFrustum.leftFace) &&
                globalAABB.IsOnOrForwardPlane(camFrustum.rightFace) &&
                globalAABB.IsOnOrForwardPlane(camFrustum.topFace) &&
                globalAABB.IsOnOrForwardPlane(camFrustum.bottomFace) &&
                globalAABB.IsOnOrForwardPlane(camFrustum.nearFace) &&
                globalAABB.IsOnOrForwardPlane(camFrustum.farFace));
    };


    static AABB GenerateAABB(const std::vector<Mesh> &meshes) {
        glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());
        for (auto &&mesh: meshes) {
            for (auto &&vertex: mesh.vertices) {
                minAABB.x = std::min(minAABB.x, vertex.position.x);
                minAABB.y = std::min(minAABB.y, vertex.position.y);
                minAABB.z = std::min(minAABB.z, vertex.position.z);
                maxAABB.x = std::max(maxAABB.x, vertex.position.x);
                maxAABB.y = std::max(maxAABB.y, vertex.position.y);
                maxAABB.z = std::max(maxAABB.z, vertex.position.z);
            }
        }
        return AABB(minAABB, maxAABB);
    }
};

#endif // !AABB_H
