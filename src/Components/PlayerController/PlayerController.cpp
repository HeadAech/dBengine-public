#include "PlayerController.h"
#include <algorithm>
#include "Components/Animator/AnimationLibrary/AnimationLibrary.h"
#include "Components/Animator/Animator.h"
#include "Components/Camera/Camera.h"
#include "Components/PhysicsBody/PhysicsBody.h"
#include "Components/ThirdPersonCamera/ThirdPersonCamera.h"
#include "GameObject/GameObject.h"
#include "InputManager/Input.h"
#include "dBengine/EngineDebug/EngineDebug.h"

PlayerController::PlayerController() {
    name = "PlayerController";
    enabled = true;
    m_attackHitbox = nullptr;
    m_isAttacking = false;
    m_attackDuration = 0.3f; // 300ms
    m_attackCooldown = 1.0f; // 1s cooldown
    m_attackTimer = 0.0f;
    m_cooldownTimer = 0.0f;
    m_currentAnimationState = PlayerAnimationState::IDLE;
    m_previousAnimationState = PlayerAnimationState::IDLE;
    m_animationsInitialized = false;
}

void PlayerController::Update(float deltaTime) {
    if (!enabled || !gameObject)
        return;

    if (!pb) {
        pb = gameObject->GetComponent<PhysicsBody>();
        if (!pb) {
            return;
        }
    }

    if (!m_animator) {
        m_animator = gameObject->GetComponent<Animator>();
        if (m_animator && !m_animationsInitialized) {
            InitAnimations();
            SetAnimTransit();
            m_animationsInitialized = true;
            std::cout << "Player animations initialized" << std::endl;
        } else if (!m_animator) {
            std::cout << "Warning: No Animator component on player" << std::endl;
        }
    }

    if (!m_attackHitbox) {
        GameObject *hitboxObj = gameObject->GetChild("AttackHitbox"); 
        if (hitboxObj) {
            m_attackHitbox = hitboxObj->GetComponent<Hitbox>();
            if (m_attackHitbox) {
            } else {
                std::cout << "No Hitbox component found on AttackHitbox!" << std::endl;
            }
        }
    }

    if (m_dashCooldownTimer > 0.0f) {
        m_dashCooldownTimer -= deltaTime;
    }

    handleMovementInput(deltaTime);
    handleAttackInput(deltaTime);
    handleDashInput(deltaTime);

    updateAttack(deltaTime);
    updateDash(deltaTime);

    updateAnimState();
}

void PlayerController::updateAnimState() {
    if (!m_animator)
        return;

    PlayerAnimationState desiredState = getAnimationState();

    if (desiredState != m_currentAnimationState) {
        transitToAnim(desiredState);
    }
}

PlayerAnimationState PlayerController::getAnimationState() {
    if (m_isAttacking) {
        return PlayerAnimationState::ATTACK;
    }

    if (pb) {
        glm::vec3 horizontalVelocity = glm::vec3(pb->velocity.x, 0.0f, pb->velocity.z);
        float speed = glm::length(horizontalVelocity);

        if (speed > 1.0) {
            return PlayerAnimationState::RUN;
        }
    }

    return PlayerAnimationState::IDLE;
}

void PlayerController::transitToAnim(PlayerAnimationState newState) {
    if (!m_animator || newState == m_currentAnimationState)
        return;

    std::string transitionName = getTransitionName(m_currentAnimationState, newState);

    if (!transitionName.empty()) {
        m_animator->PlayTransition(transitionName);
        //m_animator->SetTimeScale(0);
        m_previousAnimationState = m_currentAnimationState;
        m_currentAnimationState = newState;

        //std::cout << "Animation transition: " << static_cast<int>(m_previousAnimationState) << " -> "
        //          << static_cast<int>(m_currentAnimationState) << " (transition: " << transitionName << ")"
        //          << std::endl;
    } else {
        std::string animationName = getAnimationName(newState);
        if (!animationName.empty()) {
            m_animator->SetCurrentAnimation(animationName);
            m_previousAnimationState = m_currentAnimationState;
            m_currentAnimationState = newState;
        }
    }
}

std::string PlayerController::getTransitionName(PlayerAnimationState from, PlayerAnimationState to) {
    if (from == PlayerAnimationState::IDLE && to == PlayerAnimationState::RUN) {
        return idleToRunTransit;
    }
    if (from == PlayerAnimationState::RUN && to == PlayerAnimationState::IDLE) {
        return runToIdleTransit;
    }
    if (from == PlayerAnimationState::IDLE && to == PlayerAnimationState::ATTACK) {
        return idleToAttackTransit;
    }
    if (from == PlayerAnimationState::RUN && to == PlayerAnimationState::ATTACK) {
        return runToAttackTransit;
    }
    if (from == PlayerAnimationState::ATTACK && to == PlayerAnimationState::IDLE) {
        return attackToIdleTransit;
    }
    if (from == PlayerAnimationState::ATTACK && to == PlayerAnimationState::RUN) {
        return attackToRunTransit;
    }

    return "";
}

std::string PlayerController::getAnimationName(PlayerAnimationState state) {
    switch (state) {
        case PlayerAnimationState::IDLE:
            return idle;
        case PlayerAnimationState::RUN:
            return run;
        case PlayerAnimationState::ATTACK:
            return attack;
        default:
            return "";
    }
}

void PlayerController::InitAnimations() {
    if (!m_animator)
        return;

    AnimationLibrary &animLib = AnimationLibrary::GetInstance();

    auto idleAnim = animLib.GetAnimation(idle);
    m_animator->AddAnimation(idleAnim);

    auto runAnim = animLib.GetAnimation(run);
    m_animator->AddAnimation(runAnim);
    auto attackAnim = animLib.GetAnimation(attack);
    m_animator->AddAnimation(attackAnim);

    if (idleAnim) {
        m_animator->SetCurrentAnimation(idle);
    }
}

void PlayerController::SetAnimTransit() {
    if (!m_animator)
        return;

    AnimationLibrary &animLib = AnimationLibrary::GetInstance();

    auto idleAnim = animLib.GetAnimation(idle);
    auto runAnim = animLib.GetAnimation(run);
    auto attackAnim = animLib.GetAnimation(attack);

    if (idleAnim && runAnim) {
        AnimationTransition idleToRun;
        idleToRun.name = idleToRunTransit;
        idleToRun.duration = idleToRunTransition;
        idleToRun.animationBase = idleAnim;
        idleToRun.animationTarget = runAnim;
        m_animator->AddAnimationTransition(idleToRun);

        AnimationTransition runToIdle;
        runToIdle.name = runToIdleTransit;
        runToIdle.duration = runToIdleTransition;
        runToIdle.animationBase = runAnim;
        runToIdle.animationTarget = idleAnim;
        m_animator->AddAnimationTransition(runToIdle);
    }

    if (idleAnim && attackAnim) {
        AnimationTransition idleToAttack;
        idleToAttack.name = idleToAttackTransit;
        idleToAttack.duration = attackTransitionDuration;
        idleToAttack.animationBase = idleAnim;
        idleToAttack.animationTarget = attackAnim;
        m_animator->AddAnimationTransition(idleToAttack);

        AnimationTransition attackToIdle;
        attackToIdle.name = attackToIdleTransit;
        attackToIdle.duration = attackTransitionDuration;
        attackToIdle.animationBase = attackAnim;
        attackToIdle.animationTarget = idleAnim;
        m_animator->AddAnimationTransition(attackToIdle);
    }

    if (runAnim && attackAnim) {
        AnimationTransition runToAttack;
        runToAttack.name = runToAttackTransit;
        runToAttack.duration = attackTransitionDuration;
        runToAttack.animationBase = runAnim;
        runToAttack.animationTarget = attackAnim;
        m_animator->AddAnimationTransition(runToAttack);

        AnimationTransition attackToRun;
        attackToRun.name = attackToRunTransit;
        attackToRun.duration = attackTransitionDuration;
        attackToRun.animationBase = attackAnim;
        attackToRun.animationTarget = runAnim;
        m_animator->AddAnimationTransition(attackToRun);
    }
}

float PlayerController::getTransitionDuration(PlayerAnimationState from, PlayerAnimationState to) {
    if (to == PlayerAnimationState::ATTACK || from == PlayerAnimationState::ATTACK) {
        return attackTransitionDuration;
    }
    if (from == PlayerAnimationState::IDLE && to == PlayerAnimationState::RUN) {
        return idleToRunTransition;
    }
    if (from == PlayerAnimationState::RUN && to == PlayerAnimationState::IDLE) {
        return runToIdleTransition;
    }
    return 0.25f;
}

void PlayerController::SetThirdPersonCamera(ThirdPersonCamera *camera) { m_thirdPersonCamera = camera; }

void PlayerController::handleDashInput(float deltaTime) {
    Input &input = Input::GetInstance();

    if (input.IsActionJustPressed("left_shift") && CanDash()) {
        glm::vec3 inputDirection(0.0f);

        if (input.IsActionPressed("forward")) {
            inputDirection.z += 1.0f;
        }
        if (input.IsActionPressed("backward")) {
            inputDirection.z -= 1.0f;
        }
        if (input.IsActionPressed("left")) {
            inputDirection.x -= 1.0f;
        }
        if (input.IsActionPressed("right")) {
            inputDirection.x += 1.0f;
        }

        if (glm::length(inputDirection) == 0.0f) {
            inputDirection.z = 1.0f;
        } else {
            inputDirection = glm::normalize(inputDirection);
        }

        glm::vec3 worldDirection;
        if (m_thirdPersonCamera) {
            worldDirection = getCameraDirection(inputDirection);
        } else {
            worldDirection = getPlayerDirection(inputDirection);
        }

        PerformDash(worldDirection);
    }
}

void PlayerController::updateDash(float deltaTime) {
    if (!m_isDashing)
        return;

    m_dashTimer += deltaTime;

    if (m_dashTimer >= dashDuration) {
        endDash();
        return;
    }

    glm::vec3 dashForceVector = m_dashDirection * dashForce;
    pb->ApplyForce(dashForceVector);
}

void PlayerController::PerformDash(const glm::vec3 &direction) {
    if (!CanDash() || !pb)
        return;

    startDash(direction);
}

void PlayerController::startDash(const glm::vec3 &direction) {
    m_isDashing = true;
    m_dashTimer = 0.0f;
    m_dashCooldownTimer = dashCooldown;
    m_dashDirection = glm::normalize(direction);

    m_preDashVelocity = pb->velocity;

    glm::vec3 dashImpulse = m_dashDirection * dashSpeed * pb->mass;

    pb->velocity.x = 0.0f;
    pb->velocity.z = 0.0f;
    pb->ApplyForce(dashImpulse);

    std::cout << "Dash started" << std::endl;
}

void PlayerController::endDash() {
    m_isDashing = false;
    m_dashTimer = 0.0f;

    glm::vec3 currentVelocity = pb->velocity;
    float horizontalSpeed = glm::length(glm::vec3(currentVelocity.x, 0.0f, currentVelocity.z));

    if (horizontalSpeed > maxSpeed) {
        glm::vec3 horizontalDirection = glm::normalize(glm::vec3(currentVelocity.x, 0.0f, currentVelocity.z));
        pb->velocity.x = horizontalDirection.x * maxSpeed;
        pb->velocity.z = horizontalDirection.z * maxSpeed;
    }

    std::cout << "Dash ended" << std::endl;
}

bool PlayerController::CanDash() const { return !m_isDashing && !m_isAttacking && m_dashCooldownTimer <= 0.0f; }

float PlayerController::GetDashCooldownProgress() const {
    if (m_dashCooldownTimer <= 0.0f)
        return 1.0f;
    return 1.0f - (m_dashCooldownTimer / dashCooldown);
}

void PlayerController::handleMovementInput(float deltaTime) {
    Input &input = Input::GetInstance();
    glm::vec3 inputDirection(0.0f);

    if (input.IsActionPressed("forward")) {
        inputDirection.z += 1.0f;
    }
    if (input.IsActionPressed("backward")) {
        inputDirection.z -= 1.0f;
    }
    if (input.IsActionPressed("left")) {
        inputDirection.x -= 1.0f;
    }
    if (input.IsActionPressed("right")) {
        inputDirection.x += 1.0f;
    }

    if (glm::length(inputDirection) > 0.0f) {
        inputDirection = glm::normalize(inputDirection);
    }

    applyMovement(inputDirection, deltaTime);
}

void PlayerController::applyMovement(const glm::vec3 &inputDirection, float deltaTime) {
    if (!pb)
        return;

    glm::vec3 horizontalVelocity = glm::vec3(pb->velocity.x, 0.0f, pb->velocity.z);

    glm::vec3 worldInputDir;
    if (m_thirdPersonCamera) {
        worldInputDir = getCameraDirection(inputDirection);
    } else {
        worldInputDir = getPlayerDirection(inputDirection);
    }

    if (glm::length(inputDirection) > 0.2f) {
        glm::vec3 targetVelocity = worldInputDir * moveSpeed;
        glm::vec3 velocityDiff = targetVelocity - horizontalVelocity;
        float movementMultiplier = 1.0f;
        if (m_isDashing) {
            movementMultiplier = 0.1f;
        } else if (m_isAttacking) {
            movementMultiplier = 0.3f;
        }

        glm::vec3 accForce = velocityDiff * acceleration * movementMultiplier;

        if (glm::length(horizontalVelocity + accForce * deltaTime) > maxSpeed && !m_isDashing) {
            accForce = glm::normalize(accForce) * (maxSpeed - glm::length(horizontalVelocity)) / deltaTime;
        }

        pb->ApplyForce(glm::vec3(accForce.x, 0.0f, accForce.z) * pb->mass);

        if (!m_isDashing && !m_isAttacking) {
            applyRotation(worldInputDir, deltaTime);
        }
    } else {
        if (glm::length(horizontalVelocity) > 0.01f && !m_isDashing) {
            float velocityMagnitude = glm::length(horizontalVelocity);
            float frictionMultiplier = groundFriction + (velocityMagnitude * velocityFriction);
            
            glm::vec3 decForce = -glm::normalize(horizontalVelocity) * deceleration * frictionMultiplier * pb->mass;

            if (glm::length(decForce * deltaTime) > glm::length(horizontalVelocity * pb->mass)) {
                pb->velocity.x = 0.0f;
                pb->velocity.z = 0.0f;
            } else {
                pb->ApplyForce(decForce);
            }
        }
    }
}

glm::vec3 PlayerController::getCameraDirection(const glm::vec3 &inputDirection) {
    if (!m_thirdPersonCamera) {
        return getPlayerDirection(inputDirection);
    }
    
    glm::vec3 cameraForward = m_thirdPersonCamera->Front;
    glm::vec3 cameraRight = m_thirdPersonCamera->Right;

    cameraForward.y = 0.0f;
    cameraRight.y = 0.0f;

    if (glm::length(cameraForward) > 0.001f) {
        cameraForward = glm::normalize(cameraForward);
    } else {
        cameraForward = glm::vec3(0.0f, 0.0f, 1.0f);
    }

    if (glm::length(cameraRight) > 0.001f) {
        cameraRight = glm::normalize(cameraRight);
    } else {
        cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
    }

    glm::vec3 worldInputDir = (cameraRight * inputDirection.x) + (cameraForward * inputDirection.z);

    return worldInputDir;
}

glm::vec3 PlayerController::getPlayerDirection(const glm::vec3 &inputDirection) {
    glm::quat playerRotation = gameObject->transform.GetQuatRotation();
    glm::vec3 forward = playerRotation * glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 right = playerRotation * glm::vec3(1.0f, 0.0f, 0.0f);

    glm::vec3 worldInputDir = (right * inputDirection.x) + (forward * inputDirection.z);
    return worldInputDir;
}

void PlayerController::applyRotation(const glm::vec3 &worldInputDir, float deltaTime) {
    if (!gameObject || glm::length(worldInputDir) < 0.001f)
        return;

    glm::vec3 normalizedDir = glm::normalize(worldInputDir);
    glm::vec3 back = glm::vec3(0.0f, 0.0f, 1.0f);

    glm::quat targetRotation;

    // safety checks if return zero vector
    float dot = glm::dot(back, normalizedDir);
    if (dot < -0.9999f) {
        // vectors are opposite
        targetRotation = glm::angleAxis(glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
    } else if (dot > 0.9999f) {
        // vectors are the same
        targetRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    } else {
        glm::vec3 axis = glm::cross(back, normalizedDir);
        axis = glm::normalize(axis);
        float angle = acos(glm::clamp(dot, -1.0f, 1.0f));
        targetRotation = glm::angleAxis(angle, axis);
    }
    glm::quat currentRotation = gameObject->transform.GetQuatRotation();

    float lerpFactor = glm::clamp(15.0f * deltaTime, 0.0f, 1.0f);
    glm::quat newRotation = glm::slerp(currentRotation, targetRotation, lerpFactor);
    gameObject->transform.SetQuatRotation(newRotation);
}

void PlayerController::handleAttackInput(float deltaTime) {
    Input &input = Input::GetInstance();

    if (m_cooldownTimer > 0.0f) {
        m_cooldownTimer -= deltaTime;
    }

    if (input.IsActionJustPressed("attack") && canAttack()) {
        startAttack();
    }
}

bool PlayerController::canAttack() const {
    return !m_isAttacking && !m_isDashing && m_cooldownTimer <= 0.0f && m_attackHitbox != nullptr;
}

void PlayerController::startAttack() {
    std::cout << "Player attack started!" << std::endl;

    m_isAttacking = true;
    m_attackTimer = 0.0f;
    m_cooldownTimer = m_attackCooldown; // start cooldown

    if (m_attackHitbox) {
        m_attackHitbox->ResetForNewAttack();
        m_attackHitbox->SetActive(true);
    }
}

void PlayerController::updateAttack(float deltaTime) {
    if (!m_isAttacking)
        return;

    m_attackTimer += deltaTime;

    if (m_attackTimer >= m_attackDuration) {
        endAttack();
    }
}

void PlayerController::endAttack() {
    std::cout << "Player attack ended!" << std::endl;

    m_isAttacking = false;

    if (m_attackHitbox) {
        m_attackHitbox->SetActive(false);
    }
}
