local moveSpeed = 33.0
local acceleration = 30.0
local deceleration = 50.0
local maxSpeed = 33.0
local groundFriction = 2.0
local velocityFriction = 0.5

-- dash
local dashSpeed = 160.0
local dashDuration = 0.5
local dashCooldown = 0.6

local dashTimer = 0.0
local dashCooldownTimer = 0.0
local dashDirection = vec3.new(0, 0, 0)
local preDashVelocity = vec3.new(0, 0, 0)

--anim
local animator = nil
local animState = 0  -- idle = 0, run = 1, attack = 2, roll = 3, attack2 = 4

local isAttacking = false
local isDashing = false
local attackType = 1
local lockMovement = false

--combo
local isComboCooldownActive = false
local comboCooldown = 2.0
local comboTimer = 0.0
local comboMovementLockCooldown = 2.0

local basicDamage = 15.0

local pb = nil

local maxHealth = 100.0
local health = maxHealth

local invincibilityTimer = 0.00
local invincibilityDuration = 0.5
local invincible = false

local attackHitProgress = 0.2
local attackHitEndProgress = 0.7
local deathAnimationEndProgress = 0.99
local hitboxActivated = false

--jump combo
local isJumpComboActive = false
local jumpComboPhase = 0  -- 0 = not active, 1 = rising, 2 = falling
local jumpComboHeight = 0.0
local jumpComboMaxHeight = 40.0
local jumpComboRiseSpeed = 55.0
local jumpComboFallSpeed = 120.0
local jumpComboNormalFallSpeed = 55.0
local jumpCurrentFallSpeed = jumpComboNormalFallSpeed
local jumpComboStartY = 0.0
local jumpAttack = false
local highestReachedPoint = 0.0

local jumpComboAnimActive = false
local jumpComboAnimationEndProgress = 0.99

--fire spin combo
local isFireSpinActive = false
local fireSpinDuration = 1.0
local fireSpinTimer = 0.0
local fireSpinRotationSpeed = 1080.0 -- degrees per second
local fireSpinStartRotation = nil
local attacksLocked = false
lastInputs = {}
local attackLockTimer = 0.0
local maxAttackLockTime = 0.5
resetTimer = 2 

local startRegenStuff = false
local regenTimer = 0.0
local regenCooldown = 7.0

local regenRate = 2.0 -- how much hp regenerates
local regenInterval = 1.0 
local regenIntervalTimer = 0.0

combos = {
	{"lmb", "rmb", "lmb"},
	{"lmb", "lmb", "rmb"},
	{"lmb", "lmb", "lmb"},
	{"rmb", "lmb", "rmb"}
	--{"lmb", "rmb", "rmb"}
}

local pendingCombo = nil
local pendingComboIndex = nil

local deathScreen = nil

function onMouseMotion(offsetX, offsetY)
	return
end

function onReady()
	connect_signal("combo_detected", uuid, function(comboIndex)
		handleComboAttacks(comboIndex) 
	end)

    connect_signal("player_take_damage", uuid, function(damage)
        takeDamage(damage)
    end)

    connect_signal("combo_finished", uuid, function()
        resetAnimState()
    end)

    connect_signal("player_knockback", uuid, beginKnockback)

    pb = self():GetComponent("PhysicsBody")
    animator = self():GetComponent("Animator")
    if pb then
        pb:SetStatic(false)
    end
    if animator then
        animator:PlayTransition("run_to_idle")
    end

    health = maxHealth
    GetRef().playerHealth = 1.0

    isDead = false
    isAttacking = false
    isFireSpinActive = false
    lockMovement = false
    isJumpComboActive = false
    isDashing = false
end

knockback = false


function onUpdate(deltaTime)
    if not deathScreen then
        local scene = GetScene()
        if scene then
            deathScreen = scene:GetGameObject("Death screen")
            if deathScreen then
                deathScreen:Disable()
            end
        end
        return
    end

    if not pb then 
        pb = self():GetComponent("PhysicsBody")
        return 
    end
    if not animator then 
        animator = self():GetComponent("Animator")
        return 
    end

    if isDead then
        if animator then
            local animProgress = animator:GetAnimationProgress()
            if animProgress >= deathAnimationEndProgress then
                animator:Pause()
                deathScreen:Enable()
                Input.SetCursorLocked(false)
            end
        end
        return  
    end

    if attacksLocked then
        attackLockTimer = attackLockTimer + deltaTime
        if attackLockTimer >= maxAttackLockTime then
            attacksLocked = false
            attackLockTimer = 0.0
        end
    else
        attackLockTimer = 0.0
    end
    
    -- update dash cooldown
    if dashCooldownTimer > 0.0 then
        dashCooldownTimer = dashCooldownTimer - deltaTime
    end
   
    
    if knockback then
        applyKnockback(deltaTime)
    end
    handleMovementInput(deltaTime)
    handleDashInput(deltaTime)
    handleAttackInput()
    handleJumpCombo(deltaTime)
    handleFireSpin(deltaTime)

    checkCombo()

	expireInputs(deltaTime)

    if isComboCooldownActive then
		if comboTimer >= comboMovementLockCooldown then
			lockMovement = false
		end
		if comboTimer >= comboCooldown then
			isComboCooldownActive = false
			comboTimer = 0.0
		end
        comboTimer = comboTimer + deltaTime
	end

    if invincible then
		if invincibilityTimer >= invincibilityDuration then
			invincible = false
			invincibilityTimer = 0.0
		end
        invincibilityTimer = invincibilityTimer + deltaTime
	end

    if startRegenStuff then
        if regenTimer >= regenCooldown then
            regenIntervalTimer = regenIntervalTimer + deltaTime
            
            if regenIntervalTimer >= regenInterval then
                regenerateHealth()
                regenIntervalTimer = 0.0
            end
        else
            regenTimer = regenTimer + deltaTime
        end
    end
end

knockbackTime = 0
knockbackStrength = 0
knockbackDir = vec3.new(0,0,0)
knockbackDuration = 0

function applyKnockback(dt)
    local factor = 1.0
    if knockbackDuration > 0 then
        factor = 1.0 - (knockbackTime / knockbackDuration)
    end
    local speed = knockbackStrength * factor * dt
    -- Move player manually
    local pos = self().transform:GetGlobalPosition() -- assuming this returns a {x, y, z} table
    pos.x = pos.x + knockbackDir.x * speed
    pos.y = pos.y + knockbackDir.y * speed
    pos.z = pos.z + knockbackDir.z * speed

    self().transform:SetLocalPosition(pos)

    -- Advance time
    knockbackTime = knockbackTime + dt
    if knockbackTime >= knockbackDuration then
        knockback = false
    end
end

function beginKnockback(time, duration, strength, dirX, dirY, dirZ)
    -- Normalize direction
    knockbackDir.x = dirX
    knockbackDir.y = dirY + 0.3
    knockbackDir.z = dirZ

    knockbackTime = time
    knockbackDuration = duration 
    knockbackStrength = strength
    knockback = true
end

function handleMovementInput(deltaTime)
    if isAttacking then 
        pb.velocity = vec3.new(0, pb.velocity.y, 0)
        return 
    end
    
    if isFireSpinActive or (lockMovement and not isJumpComboActive) then
        pb.velocity = vec3.new(0, pb.velocity.y, 0)
        return 
    end
    
    local inputDirection = vec3.new(0, 0, 0)
    
    if Input.IsActionPressed("forward") then
        inputDirection.z = inputDirection.z + 1.0
    end
    if Input.IsActionPressed("backward") then
        inputDirection.z = inputDirection.z - 1.0
    end
    if Input.IsActionPressed("left") then
        inputDirection.x = inputDirection.x - 1.0
    end
    if Input.IsActionPressed("right") then
        inputDirection.x = inputDirection.x + 1.0
    end
    
    local length = math.sqrt(inputDirection.x * inputDirection.x + inputDirection.z * inputDirection.z)
    if length > 0 then
        inputDirection.x = inputDirection.x / length
        inputDirection.z = inputDirection.z / length
    end

    --idle run anim 
    if not isAttacking and not isDashing and not isFireSpinActive and animator then
        local isMoving = length > 0.2
        
        if isMoving and animState ~= 1 then
            animState = 1
            animator:PlayTransition("idle_to_run")
        end
        
        if not isMoving and animState ~= 0 then
            animState = 0
            animator:PlayTransition("run_to_idle")
        end
    end
    
    applyMovement(inputDirection, deltaTime)
end

function handleAttackInput()
    if Input.IsActionJustPressed("mouse1") and not isDashing and not lockMovement and not isJumpComboActive and not isFireSpinActive and not attacksLocked then
        addInput("lmb")
		sendChangesToUI()

        --checkCombo()

        --if pendingCombo and pendingComboIndex then
            --return
        --end

        if isAttacking then
            local playerUUID = self():GetUUID()
            emit_signal("hitbox_deactivate", playerUUID)

            if animator then
                animator:Reset() 
            end
        end

        isAttacking = true
        hitboxActivated = false
        attackType = 1

        if animator then
            if animState == 0 or animState == 4 then 
                animState = 2
                animator:PlayTransition("idle_to_attack")
            elseif animState == 1 then
                animState = 2
                animator:PlayTransition("run_to_attack")
            elseif animState == 2 then 
                animator:PlayTransition("idle_to_attack")
            end
            animator:Play() 
        end
    end
    
    if Input.IsActionJustPressed("mouse2") and not isDashing and not lockMovement and not isJumpComboActive and not isFireSpinActive and not attacksLocked then
        addInput("rmb")
		sendChangesToUI()
        
        --checkCombo()

        --if pendingCombo and pendingComboIndex then
            --return
        --end

        if isAttacking then
            local playerUUID = self():GetUUID()
            emit_signal("hitbox_deactivate", playerUUID)
            if animator then
                animator:Reset()  
            end
        end

        

        isAttacking = true
        hitboxActivated = false
        attackType = 2

        if animator then
            if animState == 0 or animState == 2 then 
                animState = 4
                animator:PlayTransition("idle_to_attack2")
            elseif animState == 1 then 
                animState = 4
                animator:PlayTransition("run_to_attack2")
            elseif animState == 4 then 
                animator:PlayTransition("idle_to_attack2")
            end
            animator:Play() 
        end
    end


    local playerUUID = self():GetUUID()

    if isAttacking and animator then
        local animProgress = animator:GetAnimationProgress()
    
        if animProgress >= attackHitProgress and animProgress < attackHitEndProgress and not hitboxActivated then
            emit_signal("hitbox_activate", playerUUID, basicDamage)
            hitboxActivated = true
        end

        if animProgress >= 0.70 then
            isAttacking = false
            hitboxActivated = false

            emit_signal("hitbox_deactivate", playerUUID)
            if animator then
                if attackType == 1 then
                    animator:PlayTransition("attack_to_idle")
                else
                    animator:PlayTransition("attack2_to_idle")
                end
            end
            animState = 0
            
            -- Sprawdź czy jest combo do wykonania po ataku
            if pendingCombo and pendingComboIndex then
                handleComboAttacks(pendingComboIndex)
                pendingCombo = nil
                pendingComboIndex = nil
            end
        end
    end
end

function handleDashInput(deltaTime)
    if Input.IsActionJustPressed("left_shift") and canDash() then
        local inputDirection = vec3.new(0, 0, 0)
        
        if Input.IsActionPressed("forward") then
            inputDirection.z = inputDirection.z + 1.0
        end
        if Input.IsActionPressed("backward") then
            inputDirection.z = inputDirection.z - 1.0
        end
        if Input.IsActionPressed("left") then
            inputDirection.x = inputDirection.x - 1.0
        end
        if Input.IsActionPressed("right") then
            inputDirection.x = inputDirection.x + 1.0
        end
        
        local inputLength = math.sqrt(inputDirection.x * inputDirection.x + inputDirection.z * inputDirection.z)
        if inputLength == 0.0 then
            inputDirection.z = 1.0
        else
            inputDirection.x = inputDirection.x / inputLength
            inputDirection.z = inputDirection.z / inputLength
        end
        
        local worldDirection = getCameraDirection(inputDirection)
		applyRotation(worldDirection, 1.0)
        
        -- start dash
        if pb then
            isDashing = true
            dashTimer = 0.0
            dashCooldownTimer = dashCooldown
            
            local dirLength = math.sqrt(worldDirection.x * worldDirection.x + worldDirection.z * worldDirection.z)
            if dirLength > 0.001 then
                dashDirection = vec3.new(worldDirection.x / dirLength, 0, worldDirection.z / dirLength)
            else
                dashDirection = vec3.new(0, 0, 1) 
            end
            
            preDashVelocity = pb.velocity
            
            local dashImpulse = vec3.new(
                dashDirection.x * dashSpeed * pb.mass,
                0,
                dashDirection.z * dashSpeed * pb.mass
            )
            
            pb.velocity = vec3.new(
                dashDirection.x * dashSpeed,
                pb.velocity.y,
                dashDirection.z * dashSpeed
            )
            
            -- anim dash
            if animator then
				animator:SetTimeScale(1.4)
                if animState == 0 then 
                    animator:PlayTransition("idle_to_roll")
                elseif animState == 1 then   
                    animator:PlayTransition("run_to_roll")
                elseif animState == 2 then  
                    animator:PlayTransition("idle_to_roll")  
                end
                animState = 3  
            end
        end
    end
    
    -- update dash
    if isDashing then
        dashTimer = dashTimer + deltaTime
        
        if dashTimer >= dashDuration then
            -- end dash
            isDashing = false
            dashTimer = 0.0

            local currentVelocity = pb.velocity
            local horizontalSpeed = math.sqrt(currentVelocity.x * currentVelocity.x + currentVelocity.z * currentVelocity.z)
            
            if horizontalSpeed > maxSpeed then
                local horizontalDirection = vec3.new(currentVelocity.x / horizontalSpeed, 0, currentVelocity.z / horizontalSpeed)
                pb.velocity = vec3.new(
                    horizontalDirection.x * maxSpeed,
                    pb.velocity.y,
                    horizontalDirection.z * maxSpeed
                )
            end
            
            -- anim dash end
            if animator then
				animator:SetTimeScale(1.0)
                --local inputDirection = getCurrentInputDirection()
                local inputLength = 0
                if inputDirection then
                    local inputLength = math.sqrt(inputDirection.x * inputDirection.x + inputDirection.z * inputDirection.z)
                end

                if inputLength > 0.2 then 
                    animator:PlayTransition("roll_to_run")
                    animState = 1  
                else  
					animator:PlayTransition("roll_to_idle") 
                    animState = 0 
                end
            end
        end
    end
end

function canDash()
    return not isDashing and dashCooldownTimer <= 0.0 and not isAttacking and not isJumpComboActive and not isFireSpinActive and not lockMovement
end

function getDashCooldownProgress()
    if dashCooldownTimer <= 0.0 then
        return 1.0
    end
    return 1.0 - (dashCooldownTimer / dashCooldown)
end

function applyMovement(inputDirection, deltaTime)
    if not pb then return end
    
    local velocity = pb.velocity
    local horizontalVelocity = vec3.new(velocity.x, 0, velocity.z)
    
    local worldInputDir = getCameraDirection(inputDirection)
    
    local inputLength = math.sqrt(inputDirection.x * inputDirection.x + inputDirection.z * inputDirection.z)
    
    if inputLength > 0.2 then
        -- sth like move_toward in godot 
        local targetVelocity = vec3.new(
            worldInputDir.x * moveSpeed,
            0,
            worldInputDir.z * moveSpeed
        )
        
        local movementMultiplier = 1.0
        if isDashing then
            movementMultiplier = 0.1 
        end
        
        local smoothing = 1.0 - math.exp(-acceleration * movementMultiplier * deltaTime)
        local newHorizontalVel = vec3.new(
            horizontalVelocity.x + (targetVelocity.x - horizontalVelocity.x) * smoothing,
            0,
            horizontalVelocity.z + (targetVelocity.z - horizontalVelocity.z) * smoothing
        )
        
        -- limit vel if not dash
        if not isDashing then
            local newSpeed = math.sqrt(newHorizontalVel.x * newHorizontalVel.x + newHorizontalVel.z * newHorizontalVel.z)
            
            if newSpeed > maxSpeed then
                local limitFactor = maxSpeed / newSpeed
                newHorizontalVel.x = newHorizontalVel.x * limitFactor
                newHorizontalVel.z = newHorizontalVel.z * limitFactor
            end
        end
        
        pb.velocity = vec3.new(newHorizontalVel.x, pb.velocity.y, newHorizontalVel.z)
        
        if not isDashing then
            applyRotation(worldInputDir, deltaTime)
        end
        
    else
        if not isDashing then
            local horizontalSpeed = math.sqrt(horizontalVelocity.x * horizontalVelocity.x + horizontalVelocity.z * horizontalVelocity.z)
            
            if horizontalSpeed > 0.01 then
                local decelerationRate = deceleration * (groundFriction + (horizontalSpeed * velocityFriction)) * deltaTime
                
                if decelerationRate >= horizontalSpeed then
                    -- instant stop
                    pb.velocity = vec3.new(0, pb.velocity.y, 0)
                else
                    local slowdownFactor = 1.0 - (decelerationRate / horizontalSpeed)
                    pb.velocity = vec3.new(
                        horizontalVelocity.x * slowdownFactor,
                        pb.velocity.y,
                        horizontalVelocity.z * slowdownFactor
                    )
                end
            end
        end
    end
end

function getCameraDirection(inputDirection)
    local scene = GetScene()
    if scene then
        local cameraObject = scene:GetGameObject("TPP Camera")
        if cameraObject then
            -- to nie dziala, nie znajduje komponentu Camera mimo, ze jest ?
            local camera = cameraObject:GetComponent("Camera")
            if camera and camera.GetYaw then
                local currentYaw = camera:GetYaw()
                if math.abs(currentYaw) > 0.1 then
                    return calculateWorldDirection(inputDirection, currentYaw)
                end
            end
            
            -- to dziala bierze pozycje obiektu kamery
            local cameraPos = cameraObject.transform:GetGlobalPosition()
            local player = scene:GetGameObject("Player")
            
            if player and cameraPos then
                local playerPos = player.transform:GetGlobalPosition()
                
                local dirX = cameraPos.x - playerPos.x 
                local dirZ = cameraPos.z - playerPos.z
                local dirLength = math.sqrt(dirX*dirX + dirZ*dirZ)
                
                if dirLength > 0.001 then
                    dirX = dirX / dirLength
                    dirZ = dirZ / dirLength
                    
                    local orbitYaw = math.deg(mathf.atan2(dirX, dirZ))
                    

                    return calculateWorldDirection(inputDirection, orbitYaw)
                end
            end
        end
    end
    
    return inputDirection
end

function calculateWorldDirection(inputDirection, orbitYaw)
    local radYaw = math.rad(orbitYaw)
    local cameraForward = vec3.new(-math.sin(radYaw), 0, -math.cos(radYaw))
    local cameraRight = vec3.new(math.cos(radYaw), 0, -math.sin(radYaw))
    
    local worldInputDir = vec3.new(
        (cameraRight.x * inputDirection.x) + (cameraForward.x * inputDirection.z),
        0,
        (cameraRight.z * inputDirection.x) + (cameraForward.z * inputDirection.z)
    )
    
    return worldInputDir
end

function applyRotation(worldInputDir, deltaTime)
    local length = math.sqrt(worldInputDir.x * worldInputDir.x + worldInputDir.z * worldInputDir.z)
    if length < 0.001 then
        return
    end
    
    local normalizedDir = vec3.new(worldInputDir.x / length, 0, worldInputDir.z / length)
    
    local targetYaw = mathf.atan2(normalizedDir.x, normalizedDir.z)
    local targetRotation = mathf.fromEuler(0, targetYaw, 0)
    
    local currentRotation = self().transform:GetQuatRotation()
    local lerpFactor = math.min(15.0 * deltaTime, 1.0)
    local newRotation = mathf.slerp(currentRotation, targetRotation, lerpFactor)
    
    self().transform:SetQuatRotation(newRotation)
end



function takeDamage(damage)
    if isDead or invincible then
        return
    end
    if not invincible then
        health = health - damage
        regenTimer = 0.0
        regenIntervalTimer = 0.0
        startRegenStuff = true
        GetRef().playerHealth = health / maxHealth
		if (health <= 0.0) then
			die()
		end
    end
end

function die()
    isDead = true
    if animator then
        animator:PlayTransition("idle_to_die")
    end
end

function handleJumpCombo(deltaTime)
    if isJumpComboActive then
        local currentPos = self().transform:GetLocalPosition()
        
        if Input.IsActionJustPressed("mouse1") then
            jumpComboPhase = 2 
            jumpCurrentFallSpeed = jumpComboFallSpeed
            jumpAttack = true
            highestReachedPoint = jumpComboHeight
        end
        
        if jumpComboPhase == 1 then
            jumpComboHeight = jumpComboHeight + jumpComboRiseSpeed * deltaTime

            if jumpComboHeight >= jumpComboMaxHeight then
                jumpComboHeight = jumpComboMaxHeight
                jumpComboPhase = 2
            end
            
            local newY = jumpComboStartY + jumpComboHeight
            self().transform:SetLocalPosition(vec3.new(currentPos.x, newY, currentPos.z))
            
        elseif jumpComboPhase == 2 then

            jumpComboHeight = jumpComboHeight - jumpCurrentFallSpeed * deltaTime

            if jumpComboHeight <= 0.0 then
                if jumpAttack then
                    local damage = highestReachedPoint * 0.8
                    emit_signal("combo4_signal", damage)
                end
                jumpComboHeight = 0.0
                jumpComboPhase = 0
                isJumpComboActive = false
                jumpAttack = false
                jumpComboAnimActive = true
    
                self().transform:SetLocalPosition(vec3.new(currentPos.x, jumpComboStartY, currentPos.z))
    
                if animator then
                    animator:PlayTransition("jump_to_idle")
                end

                emit_signal("combo_finished")
            else
                local newY = jumpComboStartY + jumpComboHeight
                self().transform:SetLocalPosition(vec3.new(currentPos.x, newY, currentPos.z))
            end
        end
    end
    -- Handle animation end
    if jumpComboAnimActive and animator then
        local animProgress = animator:GetAnimationProgress()
        if animProgress >= jumpComboAnimationEndProgress then
            jumpComboAnimActive = false
        end
    end
end

function startJumpCombo()
    if not isJumpComboActive and not isAttacking and not isDashing then
        isJumpComboActive = true
        jumpComboPhase = 1
        jumpComboHeight = 0.0
        jumpComboStartY = self().transform:GetLocalPosition().y
        jumpCurrentFallSpeed = jumpComboNormalFallSpeed
        highestReachedPoint = 0.0
        jumpComboAnimActive = true
        
        if animator then
            animator:PlayTransition("idle_to_jump")
        end
        
        return true
    end
    return false
end

function startFireSpin()
    if not isFireSpinActive and not isAttacking and not isDashing and not isJumpComboActive then
        isFireSpinActive = true
        fireSpinTimer = 0.0
        fireSpinStartRotation = self().transform:GetQuatRotation()
        
        emit_signal("combo5_signal")

        return true
    end
    return false
end

function handleFireSpin(deltaTime)
    if isFireSpinActive then        
        local rotationAmount = fireSpinRotationSpeed * fireSpinTimer
        local radians = math.rad(rotationAmount)

        local newRotation = mathf.fromEuler(0, radians, 0)
        self().transform:SetQuatRotation(newRotation)
        
        if fireSpinTimer >= fireSpinDuration then
            isFireSpinActive = false
            fireSpinTimer = 0.0

            self().transform:SetQuatRotation(fireSpinStartRotation)
            
            emit_signal("fire_spin_finished")
        end
        fireSpinTimer = fireSpinTimer + deltaTime
    end
end

function resetAnimState()
    attacksLocked = false
    attackLockTimer = 0.0
    animState = 0
end

function expireInputs(deltaTime)
	if #lastInputs == 0 then return end
	if resetTimer > 0 then
		resetTimer = resetTimer - deltaTime
	else
		lastInputs = {}
		emit_signal("clean_inputqueue", false)
	end

end

function addInput(inp)
	resetTimer = 2
	table.insert(lastInputs, 1, inp)
end

function checkCombo()
    for comboIndex, combo in ipairs(combos) do
        if #lastInputs >= #combo then
            local matched = true

            for i = 1, #combo do
                if lastInputs[i] ~= combo[#combo - i + 1] then
                    matched = false
                    break
                end
            end

            if matched then
                
                attacksLocked = true
                attackLockTimer = 0.0 
                
                if isAttacking then
                    pendingCombo = combo
                    pendingComboIndex = comboIndex
                else 
                    emit_signal("combo_detected", comboIndex)
                end
                
                lastInputs = {}
                emit_signal("clean_inputqueue", true)
                resetTimer = 2
                break
            end
        end
    end
end

function handleComboAttacks(comboIndex)
	if not isAttacking and not isDashing and not isComboCooldownActive then
		if comboIndex == 1 then
			emit_signal("combo1_signal")
		end
		if comboIndex == 2 then
			emit_signal("combo2_signal")
		end
		if comboIndex == 3 then
            emit_signal("combo3_signal")
		end
        if comboIndex == 4 then
            startJumpCombo()
		end
        if comboIndex == 5 then
			startFireSpin()
		end
        
        if not isJumpComboActive and not isFireSpinActive then
            isComboCooldownActive = true
            lockMovement = true
        end
    end
end

function sendChangesToUI()
	local test = lastInputs[1]
	if test == nil then
		emit_signal("input_update", 0)
	end
	if test == "lmb" then
		emit_signal("input_update", 1)
	end
	if test == "rmb" then
		emit_signal("input_update", 2)
	end
end

function regenerateHealth()
    if health < maxHealth and not isDead then
        health = math.min(health + regenRate, maxHealth)
        GetRef().playerHealth = health / maxHealth

        if health >= maxHealth then
            health = maxHealth
            startRegenStuff = false
            regenTimer = 0.0
            regenIntervalTimer = 0.0
            -- *CHECK* update glitch effect
        end
    end
end