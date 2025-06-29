#ifndef BOUNDING_VOLUME_H
#define BOUNDING_VOLUME_H

#include "Frustum/Frustum.h"
#include "Components/Transform/Transform.h"

struct BoundingVolume {
    virtual bool IsOnFrustum(const Frustum &camFrustum, const Transform &transform) const = 0;

    virtual bool IsOnOrForwardPlane(const Plane &plane) const = 0;

    bool IsOnFrustum(const Frustum &camFrustum) const { 
        return (
            IsOnOrForwardPlane(camFrustum.leftFace) && 
            IsOnOrForwardPlane(camFrustum.rightFace) &&
            IsOnOrForwardPlane(camFrustum.topFace) && 
            IsOnOrForwardPlane(camFrustum.bottomFace) &&
            IsOnOrForwardPlane(camFrustum.nearFace) && 
            IsOnOrForwardPlane(camFrustum.farFace)
        );
    }
};

#endif // !BOUNDING_VOLUME_H
