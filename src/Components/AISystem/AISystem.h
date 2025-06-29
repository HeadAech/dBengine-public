#ifndef AISYSTEM_H
#define AISYSTEM_H

#include <glm/glm.hpp>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "Components/AISystem/NavigationMesh/NavigationMesh.h"
#include "Components/AISystem/NavigationTarget/NavigationTarget.h"
#include "Components/AISystem/AIAgent/AIAgent.h"
#include "dBphysics/dBphysics.h"

class AIAgent;
class NavigationTarget;

struct PathNode {
    int triangleId;
    float gCost;
    float hCost;
    float fCost;
    int parent;
    glm::vec3 position;

    PathNode(int id = -1, float g = 0.0f, float h = 0.0f, int p = -1, glm::vec3 pos = glm::vec3(0.0f)) :
        triangleId(id), gCost(g), hCost(h), fCost(g + h), parent(p), position(pos) {}

    bool operator>(const PathNode &other) const { return fCost > other.fCost; }
};

class AISystem : public Component {
public:
    AISystem();
    ~AISystem();

    NavigationMesh *GetNavigationMesh();
    void SetNavigationMesh(NavigationMesh *navMesh);

    void Update(float deltaTime);

    void RegisterAgent(AIAgent *agent);
    void RemoveAgent(AIAgent *agent);
    std::unordered_set<AIAgent *> GetAgents();
    void SetAgents(std::unordered_set<AIAgent *> newAgents);

    NavigationTarget *GetTarget();
    void SetTarget(NavigationTarget *newTarget);

    void RenderNavigationMesh(Shader *shader);
    void RenderAgentPaths(Shader *shader);

    AIAgent *GetCurrentAttacker() const;
    void SetCurrentAttacker(AIAgent *attacker);

    bool HasLineOfSight(const glm::vec3 &start, const glm::vec3 &end);
    bool HasLineOfSightWithPlayer(const glm::vec3 &start, const glm::vec3 &end);
    void UpdatePathsBulk();

    void HandleAttacks(std::vector<AIAgent *> agentsInRange);

    float CalculateDistance(glm::vec3 a, glm::vec3 b) const;
    
    void InitializeCircleAngles();

    std::string m_navMeshUUID;
    std::vector<std::string> m_agentUUIDs;
    std::string m_targetUUID;

    void ResolveReferences(Scene *scene);

private:
    NavigationMesh *m_navMesh = nullptr;
    std::unordered_set<AIAgent *> agents;
    NavigationTarget *target = nullptr;
    std::unordered_set<const CollisionShape*> ignoredColliders;
    float raycastY;

    bool anyAgentNeedsPath = false;

    AIAgent *currentAttacker = nullptr;

    bool IsColliderIgnored(const CollisionShape *collider) const;
    void AddIgnoredCollider(const CollisionShape *collider);
    void RemoveIgnoredCollider(const CollisionShape *collider);

    std::unordered_map<int, PathNode> pathMap;
    std::vector<glm::vec3> SmoothPath(const std::vector<glm::vec3> &path);
    std::vector<glm::vec3> FindPath(const glm::vec3 &start, const glm::vec3 &goal);
    int FindTriangleContaining(const glm::vec3 &position);
    float Heuristic(const glm::vec3 &a, const glm::vec3 &b);

    void RenderPath(const std::vector<glm::vec3> &path, Shader *shader);
    glm::vec3 FindOptimalEdgePoint(const NavTriangle &fromTri, const NavTriangle &toTri, const glm::vec3 &previousPoint,
                                   const glm::vec3 &finalTarget);
};

#endif // AISYSTEM_H
