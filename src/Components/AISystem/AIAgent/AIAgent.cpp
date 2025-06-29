#include "Components/AISystem/AIAgent/AIAgent.h"
#include <cmath>

#define M_PI 3.14159265358979323846

AIAgent::AIAgent() :
    currentWaypointIndex(0), speed(2.0f), isPaused(false), velocity(0.0f, 0.0f, 0.0f), acceleration(0.0f, 0.0f, 0.0f),
    maxSpeed(7.0f), maxForce(10.0f), mass(1.0f), wanderAngle(0.0f), targetPos(0.0f, 0.0f, 0.0f), stoppingDistance(5.5f),
    lineOfSightDistance(30.0f), currentState(AIAgentState::IDLE), circleRadius(8.5f), circleAngle(0.0f), circleDirection(1.0) {
    name = "AIAgent";
};

AIAgent::AIAgent(glm::vec3 pos) :
    currentWaypointIndex(0), speed(1.0f), isPaused(false), agentPosition(pos),
    velocity(0.0f, 0.0f, 0.0f),
    acceleration(0.0f, 0.0f, 0.0f),
    maxSpeed(3.0f),
    maxForce(10.0f),
    mass(1.0f),
    wanderAngle(0.0f), targetPos(0.0f, 0.0f, 0.0f),
    stoppingDistance(5.5f),
    lineOfSightDistance(30.0f),
    currentState(AIAgentState::IDLE) {
    name = "AIAgent";
}

AIAgent::~AIAgent() {
}

void AIAgent::SetPath(const std::vector<glm::vec3> &newPath) {
    path = newPath;
    currentWaypointIndex = 0;
    isPaused = false;
}

void AIAgent::SetTargetPos(glm::vec3 targetPosition) {
    targetPos = targetPosition;
}

bool AIAgent::IsPaused() const { return isPaused; } 

void AIAgent::Pause() { isPaused = true; }

void AIAgent::Resume() { isPaused = false; }

void AIAgent::Update(float deltaTime) {
    if (isPaused) {
        return;
    }

    agentPosition = this->gameObject->transform.GetLocalPosition();

    acceleration = glm::vec3(0.0f);

    glm::vec3 steeringForce(0.0f);

    if (currentState == AIAgentState::ATTACK) {
        velocity = glm::vec3(0.0f);
        acceleration = glm::vec3(0.0f);

        glm::vec3 directionToTarget = targetPos - agentPosition;
        if (glm::length(directionToTarget) > 0.01f) {
            directionToTarget = glm::normalize(directionToTarget);

            float targetYaw = atan2(directionToTarget.x, directionToTarget.z);

            // Get current rotation
            glm::quat currentRotation = this->gameObject->transform.GetQuatRotation();
            float currentYaw = atan2(
                    2.0f * (currentRotation.w * currentRotation.y + currentRotation.x * currentRotation.z),
                    1.0f - 2.0f * (currentRotation.y * currentRotation.y + currentRotation.z * currentRotation.z));

            // Smoothly interpolate between current and target yaw
            float rotationSpeed = 5.0f; // Adjust this value
            float yawDiff = targetYaw - currentYaw;

            // Handle angle wrapping
            if (yawDiff > M_PI)
                yawDiff -= 2 * M_PI;
            if (yawDiff < -M_PI)
                yawDiff += 2 * M_PI;

            float newYaw = currentYaw + yawDiff * rotationSpeed * deltaTime;

            glm::quat rotation = glm::angleAxis(newYaw, glm::vec3(0.0f, 1.0f, 0.0f));

            this->gameObject->transform.SetQuatRotation(rotation);
        }
    }
    else if (currentState == AIAgentState::SEEK) {
        steeringForce += Seek(targetPos) * 1.5f;
    } else if (currentState == AIAgentState::CIRCLING) {
        steeringForce += Circle(targetPos, deltaTime) * 1.5f;
    } else if (currentState == AIAgentState::PATH_FOLLOWING) {
        steeringForce += PathFollowing() * 1.5f;
    } else if (currentState == AIAgentState::FLEE) {
        steeringForce += Flee() * 1.5f;
    } else if (currentState == AIAgentState::WANDER) {
        steeringForce += Wander() * 1.0f;
    } else if (currentState == AIAgentState::IDLE) {
        steeringForce = glm::vec3(0.0f);
        velocity = glm::vec3(0.0f);
        acceleration = glm::vec3(0.0f);
    }

    steeringForce += Separation() * 1.8f;

    acceleration += steeringForce / mass;

    velocity += acceleration * deltaTime;

    if (glm::length(velocity) > maxSpeed) {
        velocity = glm::normalize(velocity) * maxSpeed;
    }

    agentPosition += velocity * deltaTime;

    this->gameObject->transform.SetLocalPosition(agentPosition);

    if (glm::length(velocity) > 0.1f) {
        glm::vec3 direction = glm::normalize(velocity);

        float targetYaw = atan2(direction.x, direction.z);

        // Get current rotation
        glm::quat currentRotation = this->gameObject->transform.GetQuatRotation();
        float currentYaw =
                atan2(2.0f * (currentRotation.w * currentRotation.y + currentRotation.x * currentRotation.z),
                      1.0f - 2.0f * (currentRotation.y * currentRotation.y + currentRotation.z * currentRotation.z));

        // Smoothly interpolate between current and target yaw
        float rotationSpeed = 5.0f; // Adjust this value
        float yawDiff = targetYaw - currentYaw;

        // Handle angle wrapping
        if (yawDiff > M_PI)
            yawDiff -= 2 * M_PI;
        if (yawDiff < -M_PI)
            yawDiff += 2 * M_PI;

        float newYaw = currentYaw + yawDiff * rotationSpeed * deltaTime;

        glm::quat rotation = glm::angleAxis(newYaw, glm::vec3(0.0f, 1.0f, 0.0f));

        this->gameObject->transform.SetQuatRotation(rotation);
    }
}

std::vector<glm::vec3> AIAgent::GetPath() const { return path; }

void AIAgent::PrintPath() {
    for (const auto &point: path) {
        std::cout << "Waypoint: (" << point.x << ", " << point.y << ", " << point.z << ")" << std::endl;
    }
}

glm::vec3 AIAgent::GetPosition() const { return agentPosition; }
glm::vec3 AIAgent::GetVelocity() const { return velocity; }
float AIAgent::GetMaxSpeed() const { return maxSpeed; }
float AIAgent::GetMaxForce() const { return maxForce; }
float AIAgent::GetMass() const { return mass; }
float AIAgent::GetSpeed() const { return speed; }
glm::vec3 AIAgent::GetTargetPos() const { return targetPos; }
AIAgentState AIAgent::GetState() const { return currentState; }
float AIAgent::GetStoppingDistance() const { return stoppingDistance; }
float AIAgent::GetLineOfSightDistance() const { return lineOfSightDistance; }
float AIAgent::GetCircleRadius() const { return circleRadius; }
float AIAgent::GetCircleAngle() const { return circleAngle; }

void AIAgent::SetSpeed(float newSpeed) { speed = newSpeed; }
void AIAgent::SetMaxSpeed(float newMaxSpeed) { maxSpeed = newMaxSpeed; }
void AIAgent::SetMaxForce(float newMaxForce) { maxForce = newMaxForce; }
void AIAgent::SetMass(float newMass) { mass = newMass; }
void AIAgent::SetAllAgents(std::unordered_set<AIAgent *> agents) { allAgents = agents; }
void AIAgent::SetIgnoredColliders(std::unordered_set<const CollisionShape *> colliders) {
    ignoredColliders = colliders;
 }
void AIAgent::SetState(AIAgentState state) { currentState = state; }
bool AIAgent::SetStateBool(AIAgentState state) {
     if (currentState != state) {
         currentState = state;
         return true;
     }
     return false;
 }
 void AIAgent::SetStoppingDistance(float distance) { stoppingDistance = distance; }
 void AIAgent::SetLineOfSightDistance(float distance) { lineOfSightDistance = distance; }

 void AIAgent::SetPosition(glm::vec3 pos) {
    agentPosition = pos;
 }

 void AIAgent::ClearPath() {
     path.clear();
     currentWaypointIndex = 0;
 }

 GameObject *AIAgent::GetGameObject() { return gameObject; }
 std::string AIAgent::GetUUID() { return gameObject->GetUUID(); }
 void AIAgent::SetPaused(bool paused) { isPaused = paused; }
 float AIAgent::GetCircleDirection() const { return circleDirection; };
 void AIAgent::SetCircleDirection(float direction) { circleDirection = direction; };

 void AIAgent::SetCircleRadius(float radius) { circleRadius = radius; }

 void AIAgent::SetCircleAngle(float angle) { circleAngle = angle; }

glm::vec3 AIAgent::Seek(glm::vec3 target) {
    glm::vec3 desired = glm::normalize(target - agentPosition) * maxSpeed;
    glm::vec3 steer = desired - velocity;
    return Truncate(steer, maxForce);
}

glm::vec3 AIAgent::Flee() {
    glm::vec3 desired = glm::normalize(agentPosition - targetPos) * maxSpeed;
    glm::vec3 steer = desired - velocity;
    return Truncate(steer, maxForce);
}

glm::vec3 AIAgent::Separation() {
    float separationRadius = 3.0f;
    float maxForc = 15.0f;
    glm::vec3 steer(0.0f);
    int count = 0;

    auto neighbours = GetNeighborsInRadius(separationRadius);

    for (AIAgent *other: neighbours) {
        if (other == this)
            continue;

        float distance = glm::distance(agentPosition, other->GetPosition());
        if (distance > 0 && distance < separationRadius) {
            glm::vec3 diff = glm::normalize(agentPosition - other->GetPosition());
            diff /= distance; // Weight by distance
            steer += diff;
            count++;
        }
    }

    if (count > 0) {
        steer /= float(count);
        steer = glm::normalize(steer) * maxSpeed;
        steer -= velocity;
        steer = Truncate(steer, maxForc);
    }

    return steer;
}

glm::vec3 AIAgent::Wander(float wanderRadius, float wanderDistance, float maxForce) {
    glm::vec3 forwardDir = glm::length(velocity) > 0.001f ? glm::normalize(velocity) : glm::vec3(1, 0, 0);

    RaycastHit hit =
            dBphysics::GetInstance().Raycast(agentPosition, forwardDir * wanderDistance, 5.0f, ignoredColliders);
    if (hit.hit) {
        // If obstaclue is detected, retreat
        glm::vec3 retreat = -forwardDir * maxForce;
        return Truncate(retreat, maxForce) * 1.3f;
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<> dis(-0.3f, 0.3f);

    wanderAngle += dis(gen);

    // Calculate circle center in front of agent
    glm::vec3 circleCenter;
    if (glm::length(velocity) > 0.001f) {
        circleCenter = agentPosition + glm::normalize(velocity) * wanderDistance;
    } else {
        circleCenter = agentPosition + glm::vec3(1, 0, 0) * wanderDistance; // Default forward
    }

    // Calculate displacement force
    glm::vec3 displacement(cos(wanderAngle) * wanderRadius, 0, sin(wanderAngle) * wanderRadius);

    glm::vec3 wanderForce = (circleCenter + displacement) - agentPosition;
    wanderForce = Truncate(wanderForce, maxForce);

    return wanderForce;
}

glm::vec3 AIAgent::PathFollowing() {
    if (path.empty()) {
        return glm::vec3(0.0f);
    }

    // Skip waypoints that are too close to current position
    while (currentWaypointIndex < path.size()) {
        float distance = glm::distance(agentPosition, path[currentWaypointIndex]);
        if (distance > 0.5f)
            break;
        currentWaypointIndex++;
    }

    if (currentWaypointIndex >= path.size()) {
        return glm::vec3(0.0f); // Reached end of path
    }

    glm::vec3 targetWaypoint = path[currentWaypointIndex];

    // Smooth Y transition instead of sudden jumps
    float yDifference = targetWaypoint.y - agentPosition.y;
    if (abs(yDifference) > 0.1f) {
        targetWaypoint.y = agentPosition.y + (yDifference * 0.1f);
    }

    return Seek(targetWaypoint);

}

glm::vec3 AIAgent::Truncate(const glm::vec3 &vector, float maxLength) {
    float length = glm::length(vector);
    if (length > maxLength) {
        return (vector / length) * maxLength;
    }
    return vector;
}

std::unordered_set<AIAgent *> AIAgent::GetNeighborsInRadius(float radius) {
    std::unordered_set<AIAgent *> neighbors;

    for (AIAgent *other: allAgents) {
        if (other == this)
            continue;

        float distance = glm::distance(agentPosition, other->GetPosition());
        if (distance <= radius) {
            neighbors.insert(other);
        }
    }

    return neighbors;
}

glm::vec3 AIAgent::Circle(glm::vec3 target, float deltaTime) {

    glm::vec3 forwardDir = glm::length(velocity) > 0.001f ? glm::normalize(velocity) : glm::vec3(1, 0, 0);

    RaycastHit hit =
            dBphysics::GetInstance().Raycast(agentPosition, forwardDir * 3.0f, 5.0f, ignoredColliders);

    if (hit.hit) {
        // Obstacle detected, reverse circle direction
        circleDirection *= -1.0f;
    } 

    // Update circle angle based on circle speed
    circleAngle += deltaTime * circleDirection; // Assuming ~60fps

    // Keep angle in valid range
    if (circleAngle > 2 * M_PI) {
        circleAngle -= 2 * M_PI;
    }

    // Calculate desired position on circle
    glm::vec3 circleCenter = target;
    glm::vec3 desiredPos =
            circleCenter + glm::vec3(sin(circleAngle) * circleRadius, 0.0f, cos(circleAngle) * circleRadius);

    // Seek to the circle position
    glm::vec3 desired = glm::normalize(desiredPos - agentPosition) * maxSpeed * 0.5f;
    glm::vec3 steer = desired - velocity;
    return Truncate(steer, maxForce);
}