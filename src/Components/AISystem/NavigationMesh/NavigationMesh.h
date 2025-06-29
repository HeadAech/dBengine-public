#ifndef NAVIGATION_MESH_H
#define NAVIGATION_MESH_H

#include <glm/vec3.hpp>
#include <memory>
#include <vector>
#include "Components/MeshInstance/MeshInstance.h"
#include "Components/CollisionShape/CollisionShape.h"
#include <algorithm>
#include <cfloat>
#include <set>

struct NavTriangle {
    glm::vec3 a, b, c;
    glm::vec3 normal;
    glm::vec3 center;
    std::vector<int> neighbors;
    int id;

    NavTriangle(const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, int triangleId) {
        a = v0;
        b = v1;
        c = v2;
        normal = glm::normalize(glm::cross(b - a, c - a));
        center = (a + b + c) / 3.0f;
        id = triangleId;
    }
};

struct DelaunayTriangle {
    int a, b, c;
    glm::vec2 circumcenter;
    float circumradius;

    DelaunayTriangle(int p1, int p2, int p3, const std::vector<glm::vec2> &points) : a(p1), b(p2), c(p3) {
        CalculateCircumcircle(points);
    }

    void CalculateCircumcircle(const std::vector<glm::vec2> &points) {
        glm::vec2 A = points[a];
        glm::vec2 B = points[b];
        glm::vec2 C = points[c];

        float D = 2.0f * (A.x * (B.y - C.y) + B.x * (C.y - A.y) + C.x * (A.y - B.y));

        if (std::abs(D) < 1e-6f) {
            circumcenter = (A + B + C) / 3.0f;
            circumradius = 1e6f;
            return;
        }

        float ux = (A.x * A.x + A.y * A.y) * (B.y - C.y) + (B.x * B.x + B.y * B.y) * (C.y - A.y) +
                   (C.x * C.x + C.y * C.y) * (A.y - B.y);

        float uy = (A.x * A.x + A.y * A.y) * (C.x - B.x) + (B.x * B.x + B.y * B.y) * (A.x - C.x) +
                   (C.x * C.x + C.y * C.y) * (B.x - A.x);

        circumcenter.x = ux / D;
        circumcenter.y = uy / D;
        circumradius = glm::length(circumcenter - A);
    }

    bool ContainsPoint(const glm::vec2 &point) const {
        return glm::length(point - circumcenter) < circumradius - 1e-6f;
    }

    bool SharesVertex(const DelaunayTriangle &other) const {
        return (a == other.a || a == other.b || a == other.c || b == other.a || b == other.b || b == other.c ||
                c == other.a || c == other.b || c == other.c);
    }
};

struct DelaunayEdge {
    int a, b;

    DelaunayEdge(int p1, int p2) : a(std::min(p1, p2)), b(std::max(p1, p2)) {}

    bool operator==(const DelaunayEdge &other) const { return a == other.a && b == other.b; }
};

class NavigationMesh : public Component {
    public:
        NavigationMesh();
        void Generate(float maxSlopeDegrees = 45.0f);

        const std::vector<NavTriangle> &GetNavTriangles() const;
        MeshInstance* GetFloor();
        std::vector<CollisionShape *> GetObstacles();
        void SetObstacles(std::vector<CollisionShape *> temp);
        void SetFloor(MeshInstance *floorMesh);
        void AddObstacle(CollisionShape *obstacle);
        void RemoveObstacle(CollisionShape *obstacle);

        std::vector<std::string> obstaclesUUIDS;
        std::string floorUUID;
        void ResolveReferences(Scene *scene);

        void Render(Shader *shader);
    private:
        std::vector<NavTriangle> m_navTriangles;
        MeshInstance *floor = nullptr;
        std::vector<CollisionShape *> obstacles;

        std::vector<glm::vec3> ExtractFloorVertices();
        std::vector<glm::vec3> GenerateAvoidancePoints(float avoidanceRadius = 1.0f);
        float GetFloorHeightAt(const glm::vec3 &position) const;
        bool IsPointInsideTriangle(const glm::vec3 &p, const glm::vec3 &a, const glm::vec3 &b,
                                   const glm::vec3 &c) const;
        float Cross2D(const glm::vec2 &a, const glm::vec2 &b) const;

        std::vector<NavTriangle> TriangulatePoints(const std::vector<glm::vec3> &points);
        std::vector<DelaunayTriangle> DelaunayTriangulate(const std::vector<glm::vec2> &points);
        bool IsCounterClockwise(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c) const;

        bool IsPointInsideCollider(const glm::vec3 &point, CollisionShape *collider) const;
        void RemoveTrianglesInColliders();

        void BuildConnections();
        bool SharesEdge(const NavTriangle &tri1, const NavTriangle &tri2, float tolerance = 0.001f) const;
        bool PointsEqual(const glm::vec3 &p1, const glm::vec3 &p2, float tolerance = 0.001f) const;
};

#endif // NAVIGATION_MESH_H
