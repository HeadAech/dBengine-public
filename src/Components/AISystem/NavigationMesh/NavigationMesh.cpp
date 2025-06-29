#include "NavigationMesh.h"
#include <cmath>
#include <glm/geometric.hpp>

#define M_PI 3.14159265358979323846f

NavigationMesh::NavigationMesh() { name = "Navigation Mesh"; }

void NavigationMesh::Generate(float maxSlopeDegrees) {
    if (floor == nullptr)
        return;

    m_navTriangles.clear();

    // Step 1: Extract floor vertices from existing triangulation
    std::vector<glm::vec3> floorVertices = ExtractFloorVertices();

    // Step 2: Generate avoidance points around obstacles
    std::vector<glm::vec3> avoidancePoints = GenerateAvoidancePoints(1.0f);

    // Step 3: Combine floor vertices and avoidance points
    std::vector<glm::vec3> allPoints = floorVertices;
    allPoints.insert(allPoints.end(), avoidancePoints.begin(), avoidancePoints.end());

    // Step 4: Triangulate all points
    m_navTriangles = TriangulatePoints(allPoints);

    // Step 5: Remove triangles inside colliders
    RemoveTrianglesInColliders();

    // Step 6: Build connections between triangles
    BuildConnections();
}

void NavigationMesh::BuildConnections() {
    for (auto &triangle: m_navTriangles) {
        triangle.neighbors.clear();
    }

    // Check each triangle against every other triangle
    for (size_t i = 0; i < m_navTriangles.size(); ++i) {
        for (size_t j = i + 1; j < m_navTriangles.size(); ++j) {
            if (SharesEdge(m_navTriangles[i], m_navTriangles[j])) {
                m_navTriangles[i].neighbors.push_back(m_navTriangles[j].id);
                m_navTriangles[j].neighbors.push_back(m_navTriangles[i].id);
            }
        }
    }
}

bool NavigationMesh::SharesEdge(const NavTriangle &tri1, const NavTriangle &tri2, float tolerance) const {
    std::vector<glm::vec3> verts1 = {tri1.a, tri1.b, tri1.c};
    std::vector<glm::vec3> verts2 = {tri2.a, tri2.b, tri2.c};

    // Check each edge of tri1 against each edge of tri2
    for (int i = 0; i < 3; ++i) {
        glm::vec3 edge1_p1 = verts1[i];
        glm::vec3 edge1_p2 = verts1[(i + 1) % 3];

        for (int j = 0; j < 3; ++j) {
            glm::vec3 edge2_p1 = verts2[j];
            glm::vec3 edge2_p2 = verts2[(j + 1) % 3];

            // Check if edges share the same two points (in either direction)
            if ((PointsEqual(edge1_p1, edge2_p1, tolerance) && PointsEqual(edge1_p2, edge2_p2, tolerance)) ||
                (PointsEqual(edge1_p1, edge2_p2, tolerance) && PointsEqual(edge1_p2, edge2_p1, tolerance))) {
                return true;
            }
        }
    }

    return false;
}

bool NavigationMesh::PointsEqual(const glm::vec3 &p1, const glm::vec3 &p2, float tolerance) const {
    return glm::length(p1 - p2) < tolerance;
}

const std::vector<NavTriangle> &NavigationMesh::GetNavTriangles() const { return m_navTriangles; }

void NavigationMesh::ResolveReferences(Scene *scene) {
    if (!floorUUID.empty()) {
        SetFloor(scene->GetGameObjectUUID(floorUUID)->GetComponent<MeshInstance>());
    }
    for (const auto &uuid: obstaclesUUIDS) {
        CollisionShape *shape = scene->GetGameObjectUUID(uuid)->GetComponent<CollisionShape>();
        if (shape) {
            AddObstacle(shape);
        }
    }
}

void NavigationMesh::Render(Shader *shader) {
    if (m_navTriangles.empty())
        return;

    shader->Use();
    shader->SetVec3("color", glm::vec3(1.0f, 0.5f, 0.2f));

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    shader->SetMat4("model", modelMatrix);

    GLfloat prevLineWidth;
    glGetFloatv(GL_LINE_WIDTH, &prevLineWidth);
    glLineWidth(1.5f);

    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    glDisable(GL_DEPTH_TEST);

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    vertices.reserve(m_navTriangles.size() * 9);
    indices.reserve(m_navTriangles.size() * 6);

    for (size_t i = 0; i < m_navTriangles.size(); ++i) {
        const NavTriangle &triangle = m_navTriangles[i];

        vertices.push_back(triangle.a.x);
        vertices.push_back(triangle.a.y);
        vertices.push_back(triangle.a.z);

        vertices.push_back(triangle.b.x);
        vertices.push_back(triangle.b.y);
        vertices.push_back(triangle.b.z);

        vertices.push_back(triangle.c.x);
        vertices.push_back(triangle.c.y);
        vertices.push_back(triangle.c.z);

        // Add triangle edge indices (wireframe)
        GLuint baseIndex = i * 3;
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 1); 
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex);
    }

    // Create and bind VAO, VBO, EBO
    GLuint navVAO, navVBO, navEBO;

    glGenVertexArrays(1, &navVAO);
    glGenBuffers(1, &navVBO);
    glGenBuffers(1, &navEBO);

    glBindVertexArray(navVAO);

    glBindBuffer(GL_ARRAY_BUFFER, navVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, navEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);

    // Cleanup
    glDeleteVertexArrays(1, &navVAO);
    glDeleteBuffers(1, &navVBO);
    glDeleteBuffers(1, &navEBO);

    glBindVertexArray(0);

    glLineWidth(prevLineWidth);
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
}

std::vector<glm::vec3> NavigationMesh::ExtractFloorVertices() {
    std::vector<glm::vec3> vertices;
    std::set<std::pair<int, int>> uniqueVertices;

    if (!floor || !floor->model) {
        return vertices;
    }

    // Get world transform
    glm::vec3 position = floor->gameObject->transform.position;
    glm::quat rotation = floor->gameObject->transform.rotation;
    glm::vec3 scale = floor->gameObject->transform.scale;

    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
    glm::mat4 worldTransform = translationMatrix * rotationMatrix * scaleMatrix;

    for (const auto &mesh: floor->model->Meshes) {
        const auto &meshVertices = mesh.vertices;

        for (const auto &vertex: meshVertices) {
            glm::vec3 worldPos = glm::vec3(worldTransform * glm::vec4(vertex.position, 1.0f));

            // Use a simple hash to avoid duplicate vertices
            int x = (int) (worldPos.x * 1000);
            int z = (int) (worldPos.z * 1000);

            if (uniqueVertices.find({x, z}) == uniqueVertices.end()) {
                vertices.push_back(worldPos);
                uniqueVertices.insert({x, z});
            }
        }
    }

    return vertices;
}

std::vector<glm::vec3> NavigationMesh::GenerateAvoidancePoints(float avoidanceRadius) {
    std::vector<glm::vec3> avoidancePoints;

    for (CollisionShape *obstacle: obstacles) {
        if (!obstacle || !obstacle->gameObject)
            continue;

        glm::vec3 obstaclePos = obstacle->gameObject->transform.position;
        glm::vec3 obstacleScale = obstacle->gameObject->transform.scale;
        glm::vec3 obstacleSize = obstacle->GetBoxSize();
        glm::vec3 sizeScale = obstacleSize * obstacleScale;
        float floorHeight = GetFloorHeightAt(obstaclePos);

        glm::vec3 totalExtent = sizeScale * 0.5f;
        totalExtent.x += avoidanceRadius;
        totalExtent.z += avoidanceRadius;

        // Generate points at the four corners outside the collider
        glm::vec3 corners[8] = {
            glm::vec3(obstaclePos.x - totalExtent.x, floorHeight, obstaclePos.z - totalExtent.z), // Front left
            glm::vec3(obstaclePos.x + totalExtent.x, floorHeight, obstaclePos.z - totalExtent.z), // Front right
            glm::vec3(obstaclePos.x - totalExtent.x, floorHeight, obstaclePos.z + totalExtent.z), // Back left
            glm::vec3(obstaclePos.x + totalExtent.x, floorHeight, obstaclePos.z + totalExtent.z),  // Back right

            // Four midpoints on each side
            glm::vec3(obstaclePos.x, floorHeight, obstaclePos.z - totalExtent.z), // Front center
            glm::vec3(obstaclePos.x + totalExtent.x, floorHeight, obstaclePos.z), // Right center
            glm::vec3(obstaclePos.x, floorHeight, obstaclePos.z + totalExtent.z), // Back center
            glm::vec3(obstaclePos.x - totalExtent.x, floorHeight, obstaclePos.z)   
        };

        for (int i = 0; i < 8; ++i) {
            avoidancePoints.push_back(corners[i]);
        }
    }

    return avoidancePoints;
}

float NavigationMesh::GetFloorHeightAt(const glm::vec3 &position) const {
    if (!floor || !floor->model) {
        return 0.0f;
    }

    // Get world transform
    glm::vec3 floorPos = floor->gameObject->transform.position;
    glm::quat rotation = floor->gameObject->transform.rotation;
    glm::vec3 scale = floor->gameObject->transform.scale;

    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), floorPos);
    glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
    glm::mat4 worldTransform = translationMatrix * rotationMatrix * scaleMatrix;

    float closestY = position.y;
    float minDistance = FLT_MAX;

    // Find the closest triangle and interpolate height
    for (const auto &mesh: floor->model->Meshes) {
        const auto &vertices = mesh.vertices;
        const auto &indices = mesh.indices;

        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
            glm::vec3 v0 = glm::vec3(worldTransform * glm::vec4(vertices[indices[i + 0]].position, 1.0f));
            glm::vec3 v1 = glm::vec3(worldTransform * glm::vec4(vertices[indices[i + 1]].position, 1.0f));
            glm::vec3 v2 = glm::vec3(worldTransform * glm::vec4(vertices[indices[i + 2]].position, 1.0f));

            // Check if point is inside triangle (2D projection)
            if (IsPointInsideTriangle(position, v0, v1, v2)) {
                // Calculate barycentric coordinates and interpolate Y
                glm::vec3 v0v1 = v1 - v0;
                glm::vec3 v0v2 = v2 - v0;
                glm::vec3 v0p = position - v0;

                float dot00 = glm::dot(v0v2, v0v2);
                float dot01 = glm::dot(v0v2, v0v1);
                float dot02 = glm::dot(v0v2, v0p);
                float dot11 = glm::dot(v0v1, v0v1);
                float dot12 = glm::dot(v0v1, v0p);

                float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
                float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
                float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

                if (u >= 0 && v >= 0 && u + v <= 1) {
                    return v0.y + u * (v2.y - v0.y) + v * (v1.y - v0.y);
                }
            }

            // Fallback: find closest vertex
            float dist0 = glm::distance(glm::vec2(position.x, position.z), glm::vec2(v0.x, v0.z));
            float dist1 = glm::distance(glm::vec2(position.x, position.z), glm::vec2(v1.x, v1.z));
            float dist2 = glm::distance(glm::vec2(position.x, position.z), glm::vec2(v2.x, v2.z));

            if (dist0 < minDistance) {
                minDistance = dist0;
                closestY = v0.y;
            }
            if (dist1 < minDistance) {
                minDistance = dist1;
                closestY = v1.y;
            }
            if (dist2 < minDistance) {
                minDistance = dist2;
                closestY = v2.y;
            }
        }
    }

    return closestY;
}

bool NavigationMesh::IsPointInsideTriangle(const glm::vec3 &p, const glm::vec3 &a, const glm::vec3 &b,
                                           const glm::vec3 &c) const {
    glm::vec2 p2d(p.x, p.z);
    glm::vec2 a2d(a.x, a.z);
    glm::vec2 b2d(b.x, b.z);
    glm::vec2 c2d(c.x, c.z);

    float d1 = Cross2D(p2d - a2d, b2d - a2d);
    float d2 = Cross2D(p2d - b2d, c2d - b2d);
    float d3 = Cross2D(p2d - c2d, a2d - c2d);

    bool hasNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool hasPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(hasNeg && hasPos);
}

float NavigationMesh::Cross2D(const glm::vec2 &a, const glm::vec2 &b) const { return a.x * b.y - a.y * b.x; }

std::vector<NavTriangle> NavigationMesh::TriangulatePoints(const std::vector<glm::vec3> &points) {
    std::vector<NavTriangle> triangles;

    if (points.size() < 3) {
        return triangles;
    }

    // Convert 3D points to 2D for triangulation (project to XZ plane)
    std::vector<glm::vec2> points2D;
    for (const auto &point: points) {
        points2D.emplace_back(point.x, point.z);
    }

    // Perform Delaunay triangulation
    std::vector<DelaunayTriangle> delaunayTriangles = DelaunayTriangulate(points2D);

    // Convert Delaunay triangles back to NavTriangles
    int triangleId = 0;
    for (const auto &delTri: delaunayTriangles) {
        if (delTri.a < points.size() && delTri.b < points.size() && delTri.c < points.size()) {
            triangles.emplace_back(points[delTri.a], points[delTri.b], points[delTri.c], triangleId++);
        }
    }

    return triangles;
}

std::vector<DelaunayTriangle> NavigationMesh::DelaunayTriangulate(const std::vector<glm::vec2> &points) {
    if (points.size() < 3) {
        return {};
    }

    // Find bounding box
    float minX = points[0].x, maxX = points[0].x;
    float minY = points[0].y, maxY = points[0].y;

    for (const auto &p: points) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    // Create super triangle that encompasses all points
    float dx = maxX - minX;
    float dy = maxY - minY;
    float deltaMax = std::max(dx, dy) * 2.0f;

    std::vector<glm::vec2> workingPoints = points;
    workingPoints.push_back(glm::vec2(minX - deltaMax, minY - deltaMax));
    workingPoints.push_back(glm::vec2(maxX + deltaMax, minY - deltaMax));
    workingPoints.push_back(glm::vec2(minX + dx * 0.5f, maxY + deltaMax));

    int superA = workingPoints.size() - 3;
    int superB = workingPoints.size() - 2;
    int superC = workingPoints.size() - 1;

    // Initialize triangulation with super triangle
    std::vector<DelaunayTriangle> triangles;
    triangles.emplace_back(superA, superB, superC, workingPoints);

    // Add points one by one
    for (int i = 0; i < points.size(); ++i) {
        std::vector<DelaunayEdge> polygon;
        std::vector<DelaunayTriangle> newTriangles;

        // Find triangles whose circumcircle contains the new point
        for (const auto &triangle: triangles) {
            if (triangle.ContainsPoint(points[i])) {
                // Add edges of this triangle to polygon
                polygon.emplace_back(triangle.a, triangle.b);
                polygon.emplace_back(triangle.b, triangle.c);
                polygon.emplace_back(triangle.c, triangle.a);
            } else {
                newTriangles.push_back(triangle);
            }
        }

        // Remove duplicate edges (shared by two triangles)
        std::vector<DelaunayEdge> uniqueEdges;
        for (const auto &edge: polygon) {
            bool isDuplicate = false;
            for (auto it = uniqueEdges.begin(); it != uniqueEdges.end(); ++it) {
                if (*it == edge) {
                    uniqueEdges.erase(it);
                    isDuplicate = true;
                    break;
                }
            }
            if (!isDuplicate) {
                uniqueEdges.push_back(edge);
            }
        }

        // Create new triangles from the point to each edge
        for (const auto &edge: uniqueEdges) {
            newTriangles.emplace_back(edge.a, edge.b, i, workingPoints);
        }

        triangles = std::move(newTriangles);
    }

    // Remove triangles that share vertices with the super triangle
    std::vector<DelaunayTriangle> finalTriangles;
    for (const auto &triangle: triangles) {
        if (triangle.a < points.size() && triangle.b < points.size() && triangle.c < points.size()) {
            finalTriangles.push_back(triangle);
        }
    }

    return finalTriangles;
}

// Helper function to check if triangle has correct winding order
bool NavigationMesh::IsCounterClockwise(const glm::vec2 &a, const glm::vec2 &b, const glm::vec2 &c) const {
    return Cross2D(b - a, c - a) > 0;
}

bool NavigationMesh::IsPointInsideCollider(const glm::vec3 &point, CollisionShape *collider) const {
    if (!collider || !collider->gameObject)
        return false;

    glm::vec3 obstaclePos = collider->gameObject->transform.position;
    glm::vec3 obstacleScale = collider->gameObject->transform.scale;
    glm::vec3 obstacleSize = collider->GetBoxSize();
    glm::vec3 sizeScale = obstacleSize * obstacleScale;

    glm::vec3 halfExtent = sizeScale * 0.5f;

    // Check if point is within the box bounds (only X and Z axes, ignoring Y/height)
    return (point.x >= obstaclePos.x - halfExtent.x && point.x <= obstaclePos.x + halfExtent.x) &&
           (point.z >= obstaclePos.z - halfExtent.z && point.z <= obstaclePos.z + halfExtent.z);
}

void NavigationMesh::RemoveTrianglesInColliders() {
    std::vector<NavTriangle> validTriangles;

    for (const auto &triangle: m_navTriangles) {
        bool isInside = false;

        glm::vec3 center = triangle.center;

        for (CollisionShape *collider: obstacles) {
            if (IsPointInsideCollider(center, collider)) {
                isInside = true;
                break;
            }
        }

        if (!isInside) {
            validTriangles.push_back(triangle);
        }
    }

    for (size_t i = 0; i < validTriangles.size(); ++i) {
        validTriangles[i].id = i;
    }
    m_navTriangles = std::move(validTriangles);
}

MeshInstance* NavigationMesh::GetFloor() { return floor; };
std::vector<CollisionShape *> NavigationMesh::GetObstacles() { return obstacles; }
void NavigationMesh::SetObstacles(std::vector<CollisionShape *> temp) { obstacles = temp; };
void NavigationMesh::SetFloor(MeshInstance *floorMesh) {  floor = floorMesh; }
void NavigationMesh::AddObstacle(CollisionShape *obstacle) { obstacles.push_back(obstacle); };
void NavigationMesh::RemoveObstacle(CollisionShape *obstacle) {
    obstacles.erase(std::remove(obstacles.begin(), obstacles.end(), obstacle), obstacles.end());
}
