#ifndef OCTREE_H
#define OCTREE_H

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "Components/CollisionShape/CollisionShape.h"
#include "Shader/Shader.h"

class Octree {
public:
    Octree();
    ~Octree();

    struct OctreeNode {
        glm::vec3 min;
        glm::vec3 max;
        glm::vec3 center;
        std::vector<CollisionShape *> colliders;
        std::unique_ptr<OctreeNode> children[8];
        bool isLeaf = true;
        int id = 0;
        OctreeNode *parent = nullptr;
    };

    struct OctreeDebugInfo {
        int nodeId;
        int depth;
        int colliderCount;
        glm::vec3 min;
        glm::vec3 max;
    };

    void Build(const std::vector<CollisionShape *> &allColliders);
    void Insert(CollisionShape *collider);
    void GetPotentialColliders(CollisionShape *collider, std::vector<CollisionShape *> &result) const;

    void getPotentialCollidersFromNode(OctreeNode *node, CollisionShape *collider,
                                       std::vector<CollisionShape *> &resultSet) const;

    void Render(Shader *shader) const;
    std::vector<OctreeDebugInfo> GetDebugInfo() const;

    void Remove(CollisionShape *collider);
    void Update(CollisionShape *collider);

    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }

    void SetBounds(const glm::vec3 &min, const glm::vec3 &max) {
        m_worldMin = min;
        m_worldMax = max;
    }

    glm::vec3 GetMin() const { return m_worldMin; }
    glm::vec3 GetMax() const { return m_worldMax; }

    void SetMaxDepth(int depth) { m_maxDepth = depth; }
    int GetMaxDepth() const { return m_maxDepth; }

private:
    std::unique_ptr<OctreeNode> m_root;
    bool m_visible = false;
    int m_maxDepth = 2;
    mutable int m_nextNodeId = 0;

    glm::vec3 m_worldMin = glm::vec3(-200.0f, -50.0f, -200.0f);
    glm::vec3 m_worldMax = glm::vec3(200.0f, 130.0f, 200.0f);

    std::unique_ptr<OctreeNode> buildNode(const glm::vec3 &min, const glm::vec3 &max, int depth,
                                          OctreeNode *parent = nullptr);
    void insertIntoNode(OctreeNode *node, CollisionShape *collider, int depth);
    bool isColliderFullyContainedInNode(CollisionShape *collider, const OctreeNode *node) const;
    bool isColliderInNode(CollisionShape *collider, const OctreeNode *node) const;
    void getColliderBounds(CollisionShape *collider, glm::vec3 &outMin, glm::vec3 &outMax) const;
    void renderNode(const OctreeNode *node, Shader *shader) const;
    void collectDebugInfo(const OctreeNode *node, std::vector<OctreeDebugInfo> &info, int depth) const;
    void removeFromNode(OctreeNode *node, CollisionShape *collider);
};

#endif // OCTREE_H
