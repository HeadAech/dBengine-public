#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "Plane.h"
#include "GameObject/GameObject.h"

struct Frustum {
    Plane topFace;
    Plane bottomFace;

    Plane rightFace;
    Plane leftFace;

    Plane farFace;
    Plane nearFace;

    /// <summary>
    /// Creates a frustum from camera's properties.
    /// </summary>
    /// <param name="camPos">Camera's position</param>
    /// <param name="camFront">Camera's front vector</param>
    /// <param name="camRight">Camera's right vector</param>
    /// <param name="camUp">Camera's up vector</param>
    /// <param name="aspect">Aspect ratio</param>
    /// <param name="fovY">Camera's field of view</param>
    /// <param name="zNear">Near clipping plane</param>
    /// <param name="zFar">Far clipping plane</param>
    /// <returns>Frustom</returns>
    static Frustum CreateFrustumFromCamera(const glm::vec3& camPos, glm::vec3& camFront, glm::vec3& camRight, glm::vec3& camUp, float aspect, float fovY, float zNear, float zFar) {
        Frustum frustum;
        const float halfVSide = zFar * tanf(fovY * .5f);
        const float halfHSide = halfVSide * aspect;
        const glm::vec3 frontMultFar = zFar * camFront;

        frustum.nearFace = {camPos + zNear * camFront, camFront};
        frustum.farFace = {camPos + frontMultFar, -camFront};
        frustum.rightFace = {camPos, glm::cross(frontMultFar - camRight * halfHSide, camUp)};
        frustum.leftFace = {camPos, glm::cross(camUp, frontMultFar + camRight * halfHSide)};
        frustum.topFace = {camPos, glm::cross(camRight, frontMultFar - camUp * halfVSide)};
        frustum.bottomFace = {camPos, glm::cross(frontMultFar + camUp * halfVSide, camRight)};
        return frustum;
    }
};

#endif // !FRUSTUM_H
