export("max_health", 100.0)
local current_health = max_health

local pb = nil
local animator = nil

local particle_duration = 2.0
local particle_timer = particle_duration
local particles = nil
local particles_active = true

local is_dead = false

local aiagent
local aisystem
local attackArea
local isAttacking = false
local attackDuration = 2.0
local attackTimer = 0.0

local stunDuration = 0.5
local stunTimer = 0.0
local stunned = false

local animState = 0  -- idle = 0, run = 1, attack = 2
local lastAIState = nil
local mesh = nil

local damage = 2.5

function onReady()

    current_health = max_health
    
    connect_signal("hitbox_hit", uuid, onHitboxHit)
	
	connect_signal("combo_damage", uuid, function(damage, name, targetUUID)
		--takeDamage(damage, uuid, name)
        onHitboxHit(damage, name, "player", targetUUID)
	end)

	particles = self():GetChild("SpawnParticles")
	if particles then
		particles = particles:GetComponent("ParticleSystem")
		if particles then
			particles:Emit()
		end
	end

	mesh = self():GetChild("Mesh")
    if mesh then
        mesh:Disable() 
    end

    pb = self():GetComponent("PhysicsBody")
    animator = self():GetChild("Mesh"):GetComponent("Animator")
    if pb then
        pb.useGravity = true
	end
    
    if animator then
        animator:PlayAnimation("idle")
        animState = 0
    end

	particle_timer = particle_duration
end

function onUpdate(deltaTime)
	setVariables()
	
	if particles_active and particle_timer > 0 then
		particle_timer = particle_timer - deltaTime
		if particle_timer <= 0 then
			if particles then
				particles:Stop()
				aiagent:Resume()
			end
			mesh:Enable()
			particles_active = false
		
		end
	end

    if aiagent and animator and not particles_active then
        handleAnimations()
    end

    if aiagent:GetState() == AIAgentState.ATTACK then
		attack()
	end

    if isAttacking then
		if attackTimer >= attackDuration then
			isAttacking = false
			attackTimer = 0.0
		end
 	   attackTimer = attackTimer + deltaTime
    end

    
    if stunned then
		if stunTimer >= stunDuration then
			stunned = false
            aiagent:Resume()
		    stunTimer = 0.0
		end
 	   stunTimer = stunTimer + deltaTime
    end
end

function handleAnimations()
    local currentAIState = aiagent:GetState()
    
    if lastAIState ~= currentAIState or (lastAIState == AIAgentState.ATTACK and not isAttacking) then
        
        if currentAIState == AIAgentState.ATTACK and not isAttacking then
            if animState == 0 then  
                animator:PlayTransition("idle_to_attack")
            elseif animState == 1 then  
                animator:PlayTransition("run_to_attack")
            end
            animState = 2
            
        elseif currentAIState == AIAgentState.IDLE or stunned then
            if animState == 1 then 
                animator:PlayTransition("run_to_idle")
            elseif animState == 2 then
                animator:PlayTransition("attack_to_idle")
            end
            animState = 0
            
        elseif (currentAIState == AIAgentState.SEEK or 
                currentAIState == AIAgentState.PATH_FOLLOWING or 
                currentAIState == AIAgentState.WANDER) then
            if animState == 0 then 
                animator:PlayTransition("idle_to_run")
            elseif animState == 2 then 
                animator:PlayTransition("attack_to_run")
            end
            animState = 1
        end
        
        lastAIState = currentAIState
    end
    
    if isAttacking and animState == 2 and not stunned then
    end
end

function setVariables()
    if not aiagent then
            aiagent = self():GetComponent("AIAgent")
    end

	if not attackArea then
        attackArea = self():GetChild("AttackArea"):GetComponent("CollisionShape")

	end

    if not aisystem then
        aisystem = GetScene():GetGameObject("AISystem"):GetComponent("AISystem")
		if aisystem then
			aisystem:RegisterAgent(aiagent)
		end
	end

end

function onHitboxHit(damage, hitboxName, attackerUUID, targetUUID)
    
    if not damage or not targetUUID then
        return
    end
    
    local cleanTargetUUID = targetUUID:gsub("[^%w]", "")
    local cleanMyUUID = self():GetUUID():gsub("[^%w]", "")
    -- check if i was hit
    if cleanTargetUUID == cleanMyUUID then
		takeDamage(damage, attackerUUID, hitboxName)
		
    end
end

function takeDamage(damage, attackerUUID, hitboxName)
    local attacker = attackerUUID or "Unknown"
    local weapon = hitboxName or "Unknown Weapon"
    
    stunned = true
    aiagent:Pause()
	
    current_health = current_health - damage

    if current_health < 0 then
        current_health = 0
    end
    
    --emit_signal("enemy_attacked", aiagent:GetUUID())

    if current_health <= 0 then
        die(attackerUUID) 
    else
        onHurt(damage, attackerUUID)
    end
end

function die(killerUUID)
    if is_dead then
        return
    end
    is_dead = true

    if pb then
        pb:Disable()
        local cs = self():GetComponent("CollisionShape")
        if cs then
            cs:Disable()
        end
    end
    
    -- death singal
    emit_signal("enemy_death", self():GetUUID(), self().name, killerUUID)

    aisystem:RemoveAgent(aiagent)
    self():Disable()
end

function onHurt(damage, attackerUUID)
    return
end

function onDestroy()
    disconnect_signal("hitbox_hit", uuid)
end

function attack()
    if isAttacking == false and particles_active == false and stunned == false and aiagent:IsPaused() == false then
		
        isAttacking = true
        attackTimer = 0.0
        if attackArea:IsCollisionArea() then
            local gameObjects = attackArea.gameObjectsInArea
		    
            for i, obj in ipairs(gameObjects) do
				
                inArea(obj)
            end
        end
    end
end


function inArea(gameObject)
    local tag = gameObject:GetComponent("Tag")
	
	if tag then
		
		if tag.name == "Player" then
			
			emit_signal("player_take_damage", damage)
		end
	end
end