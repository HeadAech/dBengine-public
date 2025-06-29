#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>

struct Plane {
    glm::vec3 normal = {0.0f, 1.0f, 0.0f}; // unit vector
    float distance = 0.0f;

    Plane() = default;
    Plane(const glm::vec3 &p1, const glm::vec3 &norm) : normal(glm::normalize(norm)), distance(glm::dot(normal, p1)) {};

    float GetSignedDistanceToPlane(const glm::vec3 &point) const { return glm::dot(normal, point) - distance; }
};

#endif // !PLANE_H
