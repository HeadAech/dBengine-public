#ifndef SPHERE_BOUNDING_VOLUME_H
#define SPHERE_BOUNDING_VOLUME_H

#include "BoundingVolume.h"

#include "Components/Transform/Transform.h"
#include "Components/MeshInstance/MeshInstance.h"

struct SphereBoundingVolume : public BoundingVolume {
    glm::vec3 center{0.0f, 0.0f, 0.0f};
    float radius{0.0f};

    SphereBoundingVolume(const glm::vec3 &inCenter, float inRadius) :
        BoundingVolume{}, center(inCenter), radius(inRadius) {}

    bool IsOnOrForwardPlane(const Plane &plane) const override {
        return plane.GetSignedDistanceToPlane(center) > -radius;
    };

    bool IsOnFrustum(const Frustum &camFrustum, const Transform &transform) const override {
        const glm::vec3 globalScale = transform.GetGlobalScale();

        const glm::vec3 globalCenter{transform.GetModelMatrix() * glm::vec4(center, 1.0f)};

        const float maxScale = std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

        SphereBoundingVolume globalSphere(globalCenter, radius * (maxScale * 0.5f));

        return (globalSphere.IsOnOrForwardPlane(camFrustum.leftFace) &&
                globalSphere.IsOnOrForwardPlane(camFrustum.rightFace) &&
                globalSphere.IsOnOrForwardPlane(camFrustum.topFace) &&
                globalSphere.IsOnOrForwardPlane(camFrustum.bottomFace) &&
                globalSphere.IsOnOrForwardPlane(camFrustum.nearFace) &&
                globalSphere.IsOnOrForwardPlane(camFrustum.farFace));
    }

    static SphereBoundingVolume GenerateSphereBoundingVolume(const MeshInstance& meshInstance) {
        glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

        for (auto &&mesh: meshInstance.meshes) {
            for (auto &&vertex: mesh.vertices) {
                minAABB.x = std::min(minAABB.x, vertex.position.x);
                minAABB.y = std::min(minAABB.y, vertex.position.y);
                minAABB.z = std::min(minAABB.z, vertex.position.z);

                maxAABB.x = std::max(maxAABB.x, vertex.position.x);
                maxAABB.y = std::max(maxAABB.y, vertex.position.y);
                maxAABB.z = std::max(maxAABB.z, vertex.position.z);
            }
        }

        return SphereBoundingVolume((maxAABB + minAABB) * 0.5f, glm::length(minAABB - maxAABB));
    }

};
#endif // !SPHERE_BOUNDING_VOLUME_H
