#include "AISystem.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>

#define M_PI 3.14159265358979323846f

AISystem::AISystem() { 
    name = "AISystem";
    target = nullptr;
}

AISystem::~AISystem() {
    agents.clear();
}

void AISystem::Update(float deltaTime) {
    //if (!target) {
    //    return;
    //}

    //glm::vec3 targetPos = target->GetPosition();

    //std::vector<AIAgent *> agentsInRange;

    //for (auto agent: agents) {
    //    float distance = glm::distance(agent->GetPosition(), targetPos);
    //    bool hasLOS = HasLineOfSightWithPlayer(agent->GetPosition(), targetPos);
    //    float losDistance = agent->GetLineOfSightDistance();
    //    agent->SetTargetPos(targetPos);

    //    if (distance <= agent->GetStoppingDistance()) {
    //        agentsInRange.push_back(agent);
    //        if (agent != currentAttacker) {
    //            agent->SetState(AIAgentState::IDLE);
    //        }
    //        continue;
    //    } 
    //    if (!hasLOS && distance >= losDistance) {
    //        agent->SetState(AIAgentState::WANDER);
    //    } else if (hasLOS && distance < losDistance) {
    //        agent->SetState(AIAgentState::SEEK);
    //    } else if (distance < losDistance) {
    //        agent->SetState(AIAgentState::PATH_FOLLOWING);
    //        anyAgentNeedsPath = true;
    //    }
    //    }

    //HandleAttacks(agentsInRange);

    //if (anyAgentNeedsPath) {
    //    UpdatePathsBulk();
    //    anyAgentNeedsPath = false;
    //}
}

void AISystem::RegisterAgent(AIAgent *agent) {
    if (agent) {
        agents.insert(agent);
        AddIgnoredCollider(agent->gameObject->GetComponent<CollisionShape>());
        for (auto agent: agents) {
            agent->SetAllAgents(agents);
        }
    }
}

void AISystem::RemoveAgent(AIAgent *agent) {
    if (agent) {
        if (agents.find(agent) != agents.end()) {
            if (currentAttacker == agent) {
                currentAttacker = nullptr;
            }
            agents.erase(agent);
            agent->SetState(AIAgentState::IDLE);
            RemoveIgnoredCollider(agent->gameObject->GetComponent<CollisionShape>());
            for (auto *agentPtr: agents) {
                agentPtr->SetAllAgents(agents);
            }
        }
    }
}

void AISystem::SetTarget(NavigationTarget *newTarget) {
    target = newTarget;
}

void AISystem::SetNavigationMesh(NavigationMesh *navMesh) {
    m_navMesh = navMesh;
    if (navMesh && !navMesh->GetNavTriangles().empty()) {
        raycastY = navMesh->GetNavTriangles()[0].center.y + 0.5f;
    }
}

void AISystem::RenderNavigationMesh(Shader *shader) {
    //if (m_navMesh) {
    //    m_navMesh->Render(shader);
    //}
}

void AISystem::AddIgnoredCollider(const CollisionShape *collider) {
    if (collider) {
        ignoredColliders.insert(collider);
    }
    for (auto agent: agents) {
        agent->SetIgnoredColliders(ignoredColliders);
    }
}
void AISystem::RemoveIgnoredCollider(const CollisionShape *collider) { 
    ignoredColliders.erase(collider); 
    for (auto agent: agents) {
        agent->SetIgnoredColliders(ignoredColliders);
    }
}
bool AISystem::IsColliderIgnored(const CollisionShape *collider) const {
    return ignoredColliders.find(collider) != ignoredColliders.end();
}

void AISystem::UpdatePathsBulk() {
    if (!target || !m_navMesh || agents.empty()) {
        return;
    }

    glm::vec3 targetPos = target->gameObject->transform.position;

    // Find triangle containing the target
    int targetTriangle = FindTriangleContaining(targetPos);
    if (targetTriangle == -1) {
        return;
    }

    // Clear previous path map
    pathMap.clear();

    // Reverse A* from target to all reachable triangles
    std::priority_queue<PathNode, std::vector<PathNode>, std::greater<PathNode>> openSet;
    std::unordered_set<int> closedSet;

    // Start from target triangle
    PathNode startNode(targetTriangle, 0.0f, 0.0f, -1, targetPos);
    openSet.push(startNode);
    pathMap[targetTriangle] = startNode;

    const auto &triangles = m_navMesh->GetNavTriangles();

    while (!openSet.empty()) {
        PathNode current = openSet.top();
        openSet.pop();

        if (closedSet.find(current.triangleId) != closedSet.end()) {
            continue;
        }

        closedSet.insert(current.triangleId);

        // Find the triangle in our mesh
        auto triangleIt = std::find_if(triangles.begin(), triangles.end(),
                                       [current](const NavTriangle &tri) { return tri.id == current.triangleId; });

        if (triangleIt == triangles.end()) {
            continue;
        }

        const NavTriangle &currentTriangle = *triangleIt;

        for (int neighborId: currentTriangle.neighbors) {
            if (closedSet.find(neighborId) != closedSet.end()) {
                continue;
            }

            auto neighborIt = std::find_if(triangles.begin(), triangles.end(),
                                           [neighborId](const NavTriangle &tri) { return tri.id == neighborId; });

            if (neighborIt == triangles.end()) {
                continue;
            }

            const NavTriangle &neighborTriangle = *neighborIt;

            glm::vec3 edgePoint = FindOptimalEdgePoint(currentTriangle, neighborTriangle, current.position, targetPos);

            float newGCost = current.gCost + glm::distance(current.position, edgePoint);

            // Check if we found a better path to this neighbor
            auto existingPath = pathMap.find(neighborId);
            if (existingPath != pathMap.end() && existingPath->second.gCost <= newGCost) {
                continue;
            }

            // Store the edge crossing point, not the triangle center
            PathNode neighborNode(neighborId, newGCost, 0.0f, current.triangleId, edgePoint);
            pathMap[neighborId] = neighborNode;
            openSet.push(neighborNode);
        }
    }

    // Now update all agent paths
    for (AIAgent *agent: agents) {
        if (!agent || !agent->gameObject) {
            continue;
        }

        glm::vec3 agentPos = agent->gameObject->transform.position;

        // Find path using precomputed reverse A* data
        std::vector<glm::vec3> path = FindPath(agentPos, targetPos);

        // Smooth the path
        if (path.size() > 2) {
            path = SmoothPath(path);
        }

        agent->SetPath(path);
    }
}

std::vector<glm::vec3> AISystem::FindPath(const glm::vec3 &start, const glm::vec3 &goal) {
    std::vector<glm::vec3> path;

    if (!m_navMesh) {
        return path;
    }

    // Find starting triangle
    int startTriangle = FindTriangleContaining(start);
    if (startTriangle == -1) {
        return path;
    }

    // Check if we have a path to the start triangle from our reverse A*
    auto pathIt = pathMap.find(startTriangle);
    if (pathIt == pathMap.end()) {
        return path;
    }

    // Reconstruct path by following parent pointers
    std::vector<glm::vec3> reversePath;
    int currentTriangle = startTriangle;

    while (currentTriangle != -1) {
        auto nodeIt = pathMap.find(currentTriangle);
        if (nodeIt == pathMap.end()) {
            break;
        }

        reversePath.push_back(nodeIt->second.position);
        currentTriangle = nodeIt->second.parent;
    }

    // Reverse the path to get start->goal order
    path.reserve(reversePath.size());
    for (auto it = reversePath.rbegin(); it != reversePath.rend(); ++it) {
        path.push_back(*it);
    }

    // Replace first point with actual start position and last with goal
    if (!path.empty()) {
        path[0] = start;
        if (path.size() > 1) {
            path.back() = goal;
        }
    }

    return path;
}

int AISystem::FindTriangleContaining(const glm::vec3 &position) {
    if (!m_navMesh) {
        return -1;
    }

    const auto &triangles = m_navMesh->GetNavTriangles();

    for (const auto &triangle: triangles) {
        // Check if point is inside triangle using barycentric coordinates
        glm::vec3 v0 = triangle.c - triangle.a;
        glm::vec3 v1 = triangle.b - triangle.a;
        glm::vec3 v2 = position - triangle.a;

        float dot00 = glm::dot(v0, v0);
        float dot01 = glm::dot(v0, v1);
        float dot02 = glm::dot(v0, v2);
        float dot11 = glm::dot(v1, v1);
        float dot12 = glm::dot(v1, v2);

        float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

        if ((u >= 0) && (v >= 0) && (u + v <= 1)) {
            return triangle.id;
        }
    }

    // If not found, return closest triangle
    float minDistance = std::numeric_limits<float>::max();
    int closestTriangle = -1;

    for (const auto &triangle: triangles) {
        float distance = glm::distance(position, triangle.center);
        if (distance < minDistance) {
            minDistance = distance;
            closestTriangle = triangle.id;
        }
    }

    return closestTriangle;
}


bool AISystem::HasLineOfSightWithPlayer(const glm::vec3 &start, const glm::vec3 &end) {
    glm::vec3 direction = end - start;
    float distance = glm::length(direction);

    if (distance < 0.001f) {
        return true;
    }

    direction = glm::normalize(direction);

    glm::vec3 rayStart = glm::vec3(start.x, start.y, start.z);
    glm::vec3 rayEnd = glm::vec3(end.x, end.y, end.z);
    glm::vec3 rayDirection = glm::normalize(rayEnd - rayStart);
    float rayDistance = glm::distance(rayStart, rayEnd);

    RaycastHit hit = dBphysics::GetInstance().Raycast(rayStart, rayDirection, rayDistance, ignoredColliders);
    if (hit.hit && hit.collider->gameObject == target->gameObject) {
        return true;
    }

    return false;
}

bool AISystem::HasLineOfSight(const glm::vec3 &start, const glm::vec3 &end) {
    // Use physics raycast to check for obstacles
    glm::vec3 direction = end - start;
    float distance = glm::length(direction);

    if (distance < 0.001f) {
        return true;
    }

    direction = glm::normalize(direction);

    glm::vec3 rayStart = glm::vec3(start.x, raycastY, start.z);
    glm::vec3 rayEnd = glm::vec3(end.x, raycastY, end.z);
    glm::vec3 rayDirection = glm::normalize(rayEnd - rayStart);
    float rayDistance = glm::distance(rayStart, rayEnd);

    RaycastHit hit = dBphysics::GetInstance().Raycast(rayStart, rayDirection, rayDistance, ignoredColliders);
    if (hit.hit) {
        return false;
    }

    return true;
}

std::vector<glm::vec3> AISystem::SmoothPath(const std::vector<glm::vec3> &path) {
    if (path.size() <= 2) {
        return path;
    }

    std::vector<glm::vec3> smoothedPath;
    smoothedPath.push_back(path[0]);
    int currentIndex = 0;

    while (currentIndex < path.size() - 1) {
        int furthestIndex = currentIndex + 1;

        for (int i = path.size() - 1; i > currentIndex + 1; --i) {
            if (HasLineOfSight(path[currentIndex], path[i])) {
                furthestIndex = i;
                break; // Found the furthest, can stop here
            }
        }
       
        smoothedPath.push_back(path[furthestIndex]);
        currentIndex = furthestIndex;
    }

    return smoothedPath;
}

float AISystem::Heuristic(const glm::vec3 &a, const glm::vec3 &b) { return glm::distance(a, b); }

void AISystem::RenderAgentPaths(Shader *shader) {
    /*for (auto *agent: agents) {
        if (!agent->GetPath().empty() && agent->GetState() == AIAgentState::PATH_FOLLOWING && agent->GetState() != AIAgentState::WANDER) {
            RenderPath(agent->GetPath(), shader);
        }
    }*/
}

void AISystem::RenderPath(const std::vector<glm::vec3> &path, Shader *shader) {
    if (path.empty() || path.size() < 2) {
        return;
    }

    shader->Use();
    shader->SetVec3("color", glm::vec3(0.25f, 0.95f, 0.96f));

    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    for (const auto &vertex: path) {
        vertices.push_back(vertex.x);
        vertices.push_back(vertex.y + 0.1f);
        vertices.push_back(vertex.z);
    }

    for (size_t i = 0; i < path.size() - 1; i++) {
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    // Create OpenGL buffers
    GLuint pathVAO, pathVBO, pathEBO;
    glGenVertexArrays(1, &pathVAO);
    glGenBuffers(1, &pathVBO);
    glGenBuffers(1, &pathEBO);

    glBindVertexArray(pathVAO);

    glBindBuffer(GL_ARRAY_BUFFER, pathVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pathEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    GLfloat prevLineWidth;
    glGetFloatv(GL_LINE_WIDTH, &prevLineWidth);
    glLineWidth(3.0f); // Thicker line for visibility

    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    glDisable(GL_DEPTH_TEST);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    shader->SetMat4("model", modelMatrix);

    // Render path lines
    glBindVertexArray(pathVAO);
    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    glLineWidth(prevLineWidth);
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }

    // Clean up temporary buffers
    glDeleteVertexArrays(1, &pathVAO);
    glDeleteBuffers(1, &pathVBO);
    glDeleteBuffers(1, &pathEBO);
}

glm::vec3 AISystem::FindOptimalEdgePoint(const NavTriangle &fromTri, const NavTriangle &toTri,
                                         const glm::vec3 &previousPoint, const glm::vec3 &finalTarget) {
    // Find shared edge vertices
    std::vector<glm::vec3> sharedVertices;
    glm::vec3 tri1Verts[] = {fromTri.a, fromTri.b, fromTri.c};
    glm::vec3 tri2Verts[] = {toTri.a, toTri.b, toTri.c};

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (glm::distance(tri1Verts[i], tri2Verts[j]) < 0.001f) {
                sharedVertices.push_back(tri1Verts[i]);
            }
        }
    }

    if (sharedVertices.size() >= 2) {
        glm::vec3 edgeStart = sharedVertices[0];
        glm::vec3 edgeEnd = sharedVertices[1];

        glm::vec3 edgeDir = edgeEnd - edgeStart;
        float edgeLength = glm::length(edgeDir);

        if (edgeLength > 0.001f) {
            edgeDir = glm::normalize(edgeDir);

            // Calculate optimal t value
            glm::vec3 toStart = previousPoint - edgeStart;
            glm::vec3 toTarget = finalTarget - edgeStart;
            float t = glm::dot(toStart + toTarget, edgeDir * 0.5f) / edgeLength;

            // Add safety margin - avoid the very edges of the shared edge
            float safetyMargin = 0.2f; // Adjust this value (0.0 to 0.5)
            t = glm::clamp(t, safetyMargin, 1.0f - safetyMargin);

            glm::vec3 edgePoint = edgeStart + t * edgeLength * edgeDir;

            // Push the point slightly toward the triangle center for extra safety
            glm::vec3 toCenter = glm::normalize(toTri.center - edgePoint);
            edgePoint += toCenter * 0.3f; // Adjust this offset value

            return edgePoint;
        }
    }

    return toTri.center;
}

void AISystem::HandleAttacks(std::vector<AIAgent *> agentsInRange) {
    if (agentsInRange.empty()) {
        if (currentAttacker) {
            currentAttacker->SetState(AIAgentState::SEEK);
            EngineDebug::GetInstance().PrintDebug("Lost Attacker");
            currentAttacker = nullptr;
        }
        return;
    }

    // If no current attacker or current attacker is not in range, assign new one
    if (!currentAttacker ||
        std::find(agentsInRange.begin(), agentsInRange.end(), currentAttacker) == agentsInRange.end()) {

        // Choose a random agent from agentsInRange
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, static_cast<int>(agentsInRange.size()) - 1);

        currentAttacker = agentsInRange[dist(gen)];

        currentAttacker->SetState(AIAgentState::ATTACK);
        EngineDebug::GetInstance().PrintDebug("New attacker: " + currentAttacker->gameObject->name);
    }
}

void AISystem::InitializeCircleAngles() {
    if (!target || agents.empty()) {
        return;
    }

    glm::vec3 targetPos = target->GetPosition();
    std::vector<AIAgent *> circlingAgents;

    // Collect agents in CIRCLING state
    for (auto agent: agents) {
        if (agent->GetState() == AIAgentState::CIRCLING) {
            circlingAgents.push_back(agent);
        }
    }

    // Distribute them evenly around the circle
    float angleStep = (2.0f * M_PI) / std::max(1, (int) circlingAgents.size());
    for (size_t i = 0; i < circlingAgents.size(); i++) {
        AIAgent *agent = circlingAgents[i];
        // Set initial angle based on current position relative to target
        glm::vec3 toAgent = agent->GetPosition() - targetPos;
        float baseAngle = atan2(toAgent.x, toAgent.z);
        agent->SetCircleAngle(baseAngle + (i * angleStep * 0.1f)); // Slight offset for distribution
    }
}

void AISystem::ResolveReferences(Scene *scene) {
    if (!m_navMeshUUID.empty()) {
        SetNavigationMesh(scene->GetGameObjectUUID(m_navMeshUUID)->GetComponent<NavigationMesh>());
    }
    for (const auto &uuid: m_agentUUIDs) {
        AIAgent *agent = scene->GetGameObjectUUID(uuid)->GetComponent<AIAgent>();
        if (agent) {
            RegisterAgent(agent);
        }
    }
    if (!m_targetUUID.empty()) {
        SetTarget(scene->GetGameObjectUUID(m_targetUUID)->GetComponent<NavigationTarget>());
    }
}

NavigationMesh* AISystem::GetNavigationMesh() { return m_navMesh; }
std::unordered_set<AIAgent *> AISystem::GetAgents() { return agents; }
void AISystem::SetAgents(std::unordered_set<AIAgent *> newAgents) { agents = newAgents; }
NavigationTarget* AISystem::GetTarget() { return target; }
AIAgent* AISystem::GetCurrentAttacker() const { return currentAttacker; }
void AISystem::SetCurrentAttacker(AIAgent *attacker) { currentAttacker = attacker; }
float AISystem::CalculateDistance(glm::vec3 a, glm::vec3 b) const { return glm::distance(a, b); }
