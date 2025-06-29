#include "Octree.h"
#include <algorithm>
#include <glad/glad.h>
#include <limits>
#include "BoundingVolume/AABB.h"
#include "Components/CollisionShape/CollisionShape.h"

Octree::Octree() {}

Octree::~Octree() {}

void Octree::Build(const std::vector<CollisionShape *> &allColliders) {
    glm::vec3 worldMin = m_worldMin;
    glm::vec3 worldMax = m_worldMax;

    m_nextNodeId = 0;
    m_root = buildNode(worldMin, worldMax, 0);

    for (auto collider: allColliders) {
        insertIntoNode(m_root.get(), collider, 0);
    }
}

std::unique_ptr<Octree::OctreeNode> Octree::buildNode(const glm::vec3 &min, const glm::vec3 &max, int depth,
                                                      OctreeNode *parent) {
    auto node = std::make_unique<OctreeNode>();
    node->min = min;
    node->max = max;
    node->center = (min + max) * 0.5f;
    node->isLeaf = (depth >= m_maxDepth);
    node->id = m_nextNodeId++;
    node->parent = parent;

    if (!node->isLeaf) {
        for (int i = 0; i < 8; ++i) {
            glm::vec3 childMin = min;
            glm::vec3 childMax = max;

            // x-axis
            if (i & 1)
                childMin.x = node->center.x;
            else
                childMax.x = node->center.x;

            // y-axis
            if (i & 2)
                childMin.y = node->center.y;
            else
                childMax.y = node->center.y;

            // z-axis
            if (i & 4)
                childMin.z = node->center.z;
            else
                childMax.z = node->center.z;

            node->children[i] = buildNode(childMin, childMax, depth + 1, node.get());
        }
    }

    return node;
}

void Octree::Insert(CollisionShape *collider) {
    if (m_root) {
        insertIntoNode(m_root.get(), collider, 0);
    }
}

void Octree::insertIntoNode(OctreeNode *node, CollisionShape *collider, int depth) {
    if (!isColliderInNode(collider, node))
        return;

    if (node->isLeaf || depth >= m_maxDepth) {
        node->colliders.push_back(collider);
        return;
    }
    bool insertedIntoChild = false;
    for (int i = 0; i < 8; ++i) {
        if (isColliderFullyContainedInNode(collider, node->children[i].get())) {
            insertIntoNode(node->children[i].get(), collider, depth + 1);
            insertedIntoChild = true;
            break;
        }
    }
    if (!insertedIntoChild) {
        node->colliders.push_back(collider);
    }
}

void Octree::GetPotentialColliders(CollisionShape *collider, std::vector<CollisionShape *> &result) const {
    if (m_root && collider) {
        getPotentialCollidersFromNode(m_root.get(), collider, result);
    }
}

void Octree::getPotentialCollidersFromNode(OctreeNode *node, CollisionShape *collider,
                                           std::vector<CollisionShape *> &result) const {
    if (!isColliderInNode(collider, node))
        return;

    for (auto nodeCollider: node->colliders) {
        if (nodeCollider != collider) {
            result.push_back(nodeCollider);
        }
    }
    if (!node->isLeaf) {
        for (int i = 0; i < 8; ++i) {
            OctreeNode* child = node->children[i].get();
            
            if (child && isColliderInNode(collider, child)) {
                getPotentialCollidersFromNode(child, collider, result);
            }
        }
    }
}

bool Octree::isColliderFullyContainedInNode(CollisionShape *collider, const OctreeNode *node) const {
    glm::vec3 colliderMin, colliderMax;
    getColliderBounds(collider, colliderMin, colliderMax);

    return (colliderMin.x >= node->min.x && colliderMax.x <= node->max.x && colliderMin.y >= node->min.y &&
            colliderMax.y <= node->max.y && colliderMin.z >= node->min.z && colliderMax.z <= node->max.z);
}

void Octree::Remove(CollisionShape *collider) {
    if (!m_root)
        return;
    removeFromNode(m_root.get(), collider);
}

void Octree::removeFromNode(OctreeNode *node, CollisionShape *collider) {
    auto it = std::find(node->colliders.begin(), node->colliders.end(), collider);
    if (it != node->colliders.end()) {
        node->colliders.erase(it);
    }

    // Recursively check children if not a leaf
    if (!node->isLeaf) {
        for (int i = 0; i < 8; ++i) {
            if (node->children[i]) {
                removeFromNode(node->children[i].get(), collider);
            }
        }
    }
}

void Octree::Update(CollisionShape *collider) {
    Remove(collider);
    Insert(collider);
}

bool Octree::isColliderInNode(CollisionShape *collider, const OctreeNode *node) const {
    if (!collider || !collider->gameObject)
        return false;

    glm::vec3 colliderMin, colliderMax;
    getColliderBounds(collider, colliderMin, colliderMax);

    //AABB
    return !(colliderMax.x < node->min.x || colliderMin.x > node->max.x || colliderMax.y < node->min.y ||
             colliderMin.y > node->max.y || colliderMax.z < node->min.z || colliderMin.z > node->max.z);
}

void Octree::getColliderBounds(CollisionShape *collider, glm::vec3 &outMin, glm::vec3 &outMax) const {
    collider->GetWorldAABB(outMin, outMax);
}

void Octree::Render(Shader *shader) const {
    if (!m_root || !m_visible)
        return;
    shader->Use();
    renderNode(m_root.get(), shader);
}

void Octree::renderNode(const OctreeNode *node, Shader *shader) const {
    if (!node)
        return;
    glm::vec3 blue = glm::vec3(0.3f, 0.3f, 1.0f);
    shader->SetVec3("color", blue);

    std::vector<glm::vec3> vertices = {
            {node->min.x, node->min.y, node->min.z}, {node->max.x, node->min.y, node->min.z},
            {node->max.x, node->min.y, node->max.z}, {node->min.x, node->min.y, node->max.z},
            {node->min.x, node->max.y, node->min.z}, {node->max.x, node->max.y, node->min.z},
            {node->max.x, node->max.y, node->max.z}, {node->min.x, node->max.y, node->max.z}};

    std::vector<unsigned int> indices = {
            0, 1, 1, 2, 2, 3, 3, 0, // Bottom face
            4, 5, 5, 6, 6, 7, 7, 4, // Top face
            0, 4, 1, 5, 2, 6, 3, 7 // Connecting edges
    };

    static GLuint VAO = 0, VBO = 0, EBO = 0;

    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
    }

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *) 0);
    glEnableVertexAttribArray(0);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    shader->SetMat4("model", modelMatrix);

    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);

    if (!node->isLeaf) {
        for (int i = 0; i < 8; ++i) {
            if (node->children[i]) {
                renderNode(node->children[i].get(), shader);
            }
        }
    }
}

std::vector<Octree::OctreeDebugInfo> Octree::GetDebugInfo() const {
    std::vector<OctreeDebugInfo> info;

    if (!m_root)
        return info;

    collectDebugInfo(m_root.get(), info, 0);

    return info;
}

void Octree::collectDebugInfo(const OctreeNode *node, std::vector<OctreeDebugInfo> &info, int depth) const {
    if (!node)
        return;

    OctreeDebugInfo nodeInfo;
    nodeInfo.nodeId = node->id;
    nodeInfo.depth = depth;
    nodeInfo.colliderCount = node->colliders.size();
    nodeInfo.min = node->min;
    nodeInfo.max = node->max;

    info.push_back(nodeInfo);

    if (!node->isLeaf) {
        for (int i = 0; i < 8; ++i) {
            if (node->children[i]) {
                collectDebugInfo(node->children[i].get(), info, depth + 1);
            }
        }
    }
}
