#ifndef NAVIGATION_AGENT_H
#define NAVIGATION_AGENT_H

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "Components/AISystem/AISystem.h"
#include "GameObject/GameObject.h"

enum class AIAgentState { IDLE, SEEK, FLEE, WANDER, PATH_FOLLOWING, ATTACK, CIRCLING };

class AIAgent : public Component {
private:
    std::vector<glm::vec3> path;
    size_t currentWaypointIndex;

    bool isPaused;

    glm::vec3 agentPosition;

    glm::vec3 velocity;
    glm::vec3 acceleration;

    float speed;
    float maxSpeed;
    float maxForce;
    float mass;

    glm::vec3 targetPos;
    
    float lineOfSightDistance;

    float stoppingDistance;

    float circleRadius;
    float circleAngle;
    float circleDirection;

    float wanderAngle = 0.0f;

    AIAgentState currentState = AIAgentState::SEEK;
    std::unordered_set<AIAgent *> allAgents;
    std::unordered_set<const CollisionShape *> ignoredColliders;

public:
    AIAgent();
    AIAgent(glm::vec3 pos);
    ~AIAgent();

    glm::vec3 GetPosition() const;
    glm::vec3 GetVelocity() const;
    float GetMaxSpeed() const;
    float GetMaxForce() const;
    float GetMass() const;
    float GetSpeed() const;
    glm::vec3 GetTargetPos() const;
    AIAgentState GetState() const;
    float GetStoppingDistance() const;
    float GetLineOfSightDistance() const;
    GameObject *GetGameObject();
    std::string GetUUID();

    void SetSpeed(float newSpeed);
    void SetMaxSpeed(float newMaxSpeed);
    void SetMaxForce(float newMaxForce);
    void SetMass(float newMass);
    void SetTargetPos(glm::vec3 targetPosition);
    void SetState(AIAgentState state);
    bool SetStateBool(AIAgentState state);
    void SetStoppingDistance(float distance);
    void SetLineOfSightDistance(float distance);
    void SetPosition(glm::vec3 pos);
    void SetPaused(bool paused);

    void ClearPath();

    void SetAllAgents(std::unordered_set < AIAgent * > agents);
    void SetIgnoredColliders(std::unordered_set<const CollisionShape *> colliders);

    float GetCircleRadius() const;
    float GetCircleAngle() const;
    float GetCircleDirection() const;
    void SetCircleAngle(float angle);
    void SetCircleRadius(float radius);
    void SetCircleDirection(float direction);
    glm::vec3 Circle(glm::vec3 target, float deltaTime);

    glm::vec3 Seek(glm::vec3 target);
    glm::vec3 Flee();
    glm::vec3 Separation();
    glm::vec3 Wander(float wanderRadius = 2.0f, float wanderDistance = 1.0f,
                        float maxForce = 3.0f);
    glm::vec3 PathFollowing(); 
    glm::vec3 Truncate(const glm::vec3 &vector, float maxLength);
    std::unordered_set<AIAgent *> GetNeighborsInRadius(float radius);

    void SetPath(const std::vector<glm::vec3> &newPath);
    std::vector<glm::vec3> GetPath() const;
    
    bool IsPaused() const;

    void Pause();
    void Resume();

    void Update(float deltaTime);

    void PrintPath();
};

#endif // NAVIGATION_AGENT_H
