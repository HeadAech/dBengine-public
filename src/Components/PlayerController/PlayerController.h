#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H
#include <Components/Hitbox/Hitbox.h>
#include <glm/glm.hpp>
#include "Component/Component.h"

class PhysicsBody;
class Camera;
class ThirdPersonCamera;
class Animator;

enum class PlayerAnimationState { IDLE, RUN, ATTACK };

class PlayerController : public Component {
public:
    PlayerController();
    ~PlayerController() override = default;
    void Update(float deltaTime) override;

    float moveSpeed = 27.0f;
    float acceleration = 30.0f;
    float deceleration = 50.0f;
    float maxSpeed = 15.0f;
    float dashSpeed = 25.0f;
    float dashDuration = 0.2f;
    float dashCooldown = 1.0f;
    float dashForce = 800.0f;
    float groundFriction = 2.0f;
    float velocityFriction = 0.5f;

    std::string idle = "idle";
    std::string run = "run";
    std::string attack = "attack";

    std::string idleToRunTransit = "idle_to_run";
    std::string runToIdleTransit = "run_to_idle";
    std::string idleToAttackTransit = "idle_to_attack";
    std::string runToAttackTransit = "run_to_attack";
    std::string attackToIdleTransit = "attack_to_idle";
    std::string attackToRunTransit = "attack_to_run";

    float idleToRunTransition = 0.2f;
    float runToIdleTransition = 0.3f;
    float attackTransitionDuration = 0.15f;

    void SetThirdPersonCamera(ThirdPersonCamera *camera);
    ThirdPersonCamera *GetThirdPersonCamera() const { return m_thirdPersonCamera; }
    void PerformDash(const glm::vec3 &direction);
    bool CanDash() const;
    bool IsDashing() const { return m_isDashing; }
    float GetDashCooldownProgress() const;
    bool IsAttacking() const { return m_isAttacking; }
    bool CanAttack() const { return canAttack(); }

    PlayerAnimationState GetCurrentAnimationState() const { return m_currentAnimationState; }

    void InitAnimations();
    void SetAnimTransit();

private:
    PhysicsBody *pb = nullptr;
    ThirdPersonCamera *m_thirdPersonCamera = nullptr;
    Animator *m_animator = nullptr;

    PlayerAnimationState m_currentAnimationState = PlayerAnimationState::IDLE;
    PlayerAnimationState m_previousAnimationState = PlayerAnimationState::IDLE;
    bool m_animationsInitialized = false;

    void handleMovementInput(float deltaTime);
    void applyMovement(const glm::vec3 &inputDirection, float deltaTime);
    void applyRotation(const glm::vec3 &worldInputDir, float deltaTime);
    glm::vec3 getCameraDirection(const glm::vec3 &inputDirection);
    glm::vec3 getPlayerDirection(const glm::vec3 &inputDirection);
    glm::vec3 currVelocity = glm::vec3(0.0f);

    void handleAttackInput(float deltaTime);
    bool canAttack() const;
    void startAttack();
    void updateAttack(float deltaTime);
    void endAttack();

    void handleDashInput(float deltaTime);
    void updateDash(float deltaTime);
    void startDash(const glm::vec3 &direction);
    void endDash();

    void updateAnimState();
    void transitToAnim(PlayerAnimationState newState);
    PlayerAnimationState getAnimationState();
    std::string getTransitionName(PlayerAnimationState from, PlayerAnimationState to);
    std::string getAnimationName(PlayerAnimationState state);
    float getTransitionDuration(PlayerAnimationState from, PlayerAnimationState to);

    Hitbox *m_attackHitbox;
    bool m_isAttacking;
    float m_attackDuration;
    float m_attackTimer;
    float m_attackCooldown;
    float m_cooldownTimer;

    bool m_isDashing = false;
    float m_dashTimer = 0.0f;
    float m_dashCooldownTimer = 0.0f;
    glm::vec3 m_dashDirection = glm::vec3(0.0f);
    glm::vec3 m_preDashVelocity = glm::vec3(0.0f);
};

#endif // PLAYER_CONTROLLER_H
