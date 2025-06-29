-- testing purposes only
export("enabled", false)

export("initialPos", 0, -1.3, 0)
export("speed", 150)

local lerpColor
--function connectSignals()
--    signals.Timer_OnTimeout:connect(component_uuid .. "_onTimeout", function()
--        lerpColor = true
--    end, "onBeat")
--end

--function disconnectSignals()
--    signals.Timer_OnTimeout:disconnect(component_uuid .. "_onTimeout")
--end

-- variables
-- idle = 0 | run = 1 | stun = 2 | guitar = 3
local States = {
    Idle = 0,
    Run = 1,
    Stun = 2,
    Guitar = 3
}

local state = States.Idle

local animator

local player
local rushParticles
local teleportParticles

local thunder
local thunderParticles

local thunderTracer
local thunderTracerParticles

local pathObj
local pathMesh

local audioSource

local phaseNumber = 1

local phase1_AttackIdx = 1

-- path colors

local pathSeekColor = vec3.new(1, 1, 0)
local pathLockedColor = vec3.new(1, 0, 0)

--local speed = 10

local rushDirection = vec3.new()

local readyToRush = false
local isStunned = false
local rushing = false

local vulnerable = false

local speakerCounter = 0

local thunderReady = false

local maxThunderStrikes = 5
local thunderStrikes = 0


-- timers

local rushWait_Cooldown = 3
local rushWait_Timer = rushWait_Cooldown

local stunned_Cooldown = 7
local stunned_Timer = stunned_Cooldown

local locked_Duration = 0.1
local locked_Timer = locked_Duration

local rush_Duration = 0.6
local rush_Timer = rush_Duration

local thunderTracer_Duration = 0.5
local thunderTracer_Timer = thunderTracer_Duration

local thunder_Cooldown = 0.3
local thunder_Timer = thunder_Cooldown

-- healthbar

local maxHealth = 200
local health = maxHealth

local healthbar 
local healthbarBG

-- phase 2 indicators

local indicator1
local indicator2
local indicator3
local indicator4

local indicators
local indicatorsParticles = {}

local firePillar
local firePillarParticles

-- hitbox

local hitbox
local canAttack = true

local damage_Cooldown = 3
local damage_Timer = damage_Cooldown

local fleePoints = {
    vec3.new(-31.890516, 18.396864, 10.547667),
    vec3.new(-23.061596, 18.396828, -31.290688),
    vec3.new(16.368673, 18.396944, -33.225029),
    vec3.new(39.999676, 18.397081, -4.324487),
    vec3.new(25.482090, 18.396898, 32.463158),
    vec3.new(-17.894745, 18.396807, 32.399311)
}

local justFirst = true

local god = false
local invulDuration = 1
local invulTimer = 0

-- Executes when GameObject enters the scene
function onReady()
    
	enabled = true
    connect_signal("boss_hit_speaker", uuid, function()
        if phaseNumber == 1 then
            stun()
        end
    end)
    connect_signal("hitbox_hit", uuid, onHitboxHit)
    emit_signal("reset_speaker_state")

    connect_signal("combo_damage", uuid, function(damage, name, targetUUID)
        onHitboxHit(damage, name, "player", targetUUID)
    end)

    self().transform:SetLocalPosition(vec3.new(-24.730000, 18.396944, 8.800000))

    connect_signal("player_died", uuid, function()
        enabled = false
    end)
end

-- Executes on each frame
function onUpdate(deltaTime)

    -- for testing purposes, controlled by inspector
    if not enabled then return end

    if justFirst then
        justFirst = false
        setState(States.Idle)
    end

    setVariables()

    if not indicators then
        if indicator1 and indicator2 and indicator3 and indicator4 then
            indicators = {indicator1, indicator2, indicator3, indicator4}
        end
    end

    if #indicatorsParticles == 0 then 
        for i = 1,4 do 
            local a = indicators[i]:GetChild("Particles")
            local comp = a:GetComponent("ParticleSystem")
            table.insert(indicatorsParticles, i, comp)
            if comp then
                comp:Stop()
            end
        end
    end

    if health < 60 and phaseNumber ~= 2 then
        phaseNumber = 2
    end

    processAttack(deltaTime)

    if phaseNumber == 1 then
        phase1(deltaTime)
    elseif phaseNumber == 2 then
        phase2(deltaTime)
    end

    if invulTimer > 0 then
        invulTimer = invulTimer - deltaTime
        god = true
    else
        god = false
    end
    
    GetRef().bossHealth = health / maxHealth
end

function processAttack(deltaTime)
    if canAttack then
        if hitbox then
            if hitbox:IsCollisionArea() then
                local gameObjects = hitbox.gameObjectsInArea
                for i, obj in ipairs(gameObjects) do
                    local tag = obj:GetComponent("Tag")
                    if tag then
                        if tag.name == "Player" then
                            damagePlayer()
                            canAttack = false
                            damage_Timer = damage_Cooldown
                            return
                        end
                    end
                end
            end
        end
    else 
        if damage_Timer <= 0 then
            canAttack = true
            damage_Timer = damage_Cooldown
        else
            damage_Timer = damage_Timer - deltaTime
        end
    end
end

function damagePlayer()
    emit_signal("player_take_damage", 10)
    if rushDirection then
       emit_signal("player_knockback", 0, 0.4, 4.0, rushDirection.x, rushDirection.y, rushDirection.z)
    end
    
end

function setState(s)
    if state == s then
        return
    end

    local oldState = state

    if state ~= s then
        state = s
    end

    -- manage animations

    if oldState == States.Idle and state == States.Run then
        animator:PlayTransition("idle-to-run")
        return
    end

    if oldState == States.Run and state == States.Idle then
        animator:PlayTransition("run-to-idle")
        return
    end

    if oldState == States.Run and state == States.Stun then
       animator:PlayTransition("run-to-stun")
       return
    end

    if oldState == States.Stun and state == States.Idle then
        animator:PlayTransition("stun-to-idle")
        return
    end

    if oldState == States.Idle and state == States.Guitar then
        animator:PlayTransition("idle-to-guitar")
        return
    end

    if state == States.Idle then
        animator:PlayTransition("stun-to-idle")
        return
    end

    if state == States.Run then
        animator:PlayTransition("idle-to-run")
        return
    end

    if state == States.Stun then
        animator:PlayTransition("run-to-stun")
        return
    end

    if state == States.Guitar then
        animator:PlayTransition("idle-to-guitar")
        return
    end

end

-- helper functions

function setVariables() 
    if player and rushParticles and teleportParticles and thunder and 
    thunderTracer and animator and pathObj and pathMesh
    and indicator1 and indicator2 and indicator3 and indicator4 and firePillar and hitbox
    then 
        return 
    end

    local scene
    if not player or not rushParticles or not thunder or not thunderTracer or not indicator1 or not indicator2 or not indicator3 or not indicator4 or not firePillar then
        scene = GetScene()
    end

    if not player then 
        player = scene:GetGameObject("Player")
    end

    if not rushParticles then
        local particlesChild = GetChild("Rush Particles")
        if particlesChild then
            rushParticles = particlesChild:GetComponent("ParticleSystem")
        end
    end

    if not teleportParticles then
        local teleportChild = GetChild("Teleport Particles")
        if teleportChild then
            teleportParticles = teleportChild:GetComponent("ParticleSystem")
        end
    end

    if not thunder then
        thunder = scene:GetGameObject("Thunder")
        if thunder then
            thunderParticles = thunder:GetComponent("ParticleSystem")
            if not thunderTracer then
                thunderTracer = thunder:GetChild("Tracer")
                if thunderTracer then
                    thunderTracerParticles = thunderTracer:GetComponent("ParticleSystem")
                end
            end
        end
    end

    if not animator then
        local mesh = GetChild("Mesh")
        if mesh then
            animator = mesh:GetComponent("Animator")
        end
    end

    if not audioSource then
        audioSource = self():GetComponent("AudioSource")
    end

    if not pathObj then
        pathObj = GetChild("Path")
        if pathObj then
            pathMesh = pathObj:GetComponent("MeshInstance")
            oldScatter = pathMesh.scattering
        end
    end

    if not healthbar then
        local healthBarObj = GetChild("HealthBar")
        if healthBarObj then
            healthbar = healthBarObj:GetComponent("Button")
            if not healthbarBG then
                healthbarBG = healthBarObj:GetChild("BG"):GetComponent("Button")
            end
        end
    end

    if not indicator1 or not indicator2 or not indicator3 or not indicator4 or not firePillar then
        local indicators = scene:GetGameObject("Phase 2 Indicators")

        if indicators then
            if not indicator1 then
                indicator1 = indicators:GetChild("Indicator1")
            end
            if not indicator2 then
                indicator2 = indicators:GetChild("Indicator2")
            end
            if not indicator3 then
                indicator3 = indicators:GetChild("Indicator3")
            end
            if not indicator4 then
                indicator4 = indicators:GetChild("Indicator4")
            end

            if not firePillar then
                firePillar = indicators:GetChild("FirePillar")
                firePillarParticles = firePillar:GetComponent("ParticleSystem")
                if firePillarParticles then
                    firePillarParticles:Stop()
                end
            end
        end
    end

    if not hitbox then
        local g = GetChild("Hitbox")
        if g then
            hitbox = g:GetComponent("CollisionShape")
        end
    end
end

function normalize(x, y, z)
    local length = math.sqrt(x * x + y * y + z * z)
    if length == 0 then
        return 0, 0, 0
    end
    return vec3.new(x / length, y / length, z / length)
end

function atan2(y, x)
    if x > 0 then
        return math.atan(y / x)
    elseif x < 0 then
        if y >= 0 then
            return math.atan(y / x) + math.pi
        else
            return math.atan(y / x) - math.pi
        end
    elseif x == 0 then
        if y > 0 then
            return math.pi / 2
        elseif y < 0 then
            return -math.pi / 2
        else
            return 0 -- undefined, but we return 0
        end
    end
end

function rotateTowardsDirection(direction, deltaTime, lerpSpeed)
    -- Prevent NaN on zero vector
    if direction.x == 0 and direction.z == 0 then
        return 
    end

    local yaw = atan2(direction.x, direction.z)
    local targetRotation = mathf.fromEuler(0, yaw, 0)

    local currentRotation = self().transform:GetQuatRotation()

    -- Interpolate between current and target rotation
    local t = mathf.clamp(lerpSpeed * deltaTime, 0, 1) -- Interpolation factor

    local newRotation = mathf.slerp(currentRotation, targetRotation, t)

    -- Apply the interpolated rotation
    self().transform:SetQuatRotation(newRotation)
end

function randomPositionInRadius(center, radius)
    local angle = math.random() * 2 * math.pi
    local distance = math.sqrt(math.random()) * radius  -- sqrt for uniform distribution
    local height = (math.random() * 2 - 1) * radius      -- random Y offset in [-radius, radius]

    local x = center.x + math.cos(angle) * distance
    local y = center.y + height
    local z = center.z + math.sin(angle) * distance

    return vec3.new(x, y, z)
end

--

function updateHealthBar()
    if not healthbar then return end
    healthbar.size.x = health * 3

    healthbarBG.size.x = maxHealth * 3

    
end

-- phase 1 

function phase1(deltaTime)

    if phase1_AttackIdx == 1 then
        -- bumped go to stunned func
        if isStunned then
            stunned(deltaTime)
            return
        end

        rushingAttack(deltaTime)

        

        -- ??? fix
        if speakerCounter == 2 then
            phase1_AttackIdx = 2
            speakerCounter = 0
        end

    elseif phase1_AttackIdx == 2 then

        if thunder_Timer <= 0 then
            if not thunderReady then
                prepareThunder(deltaTime)
            end
        else
            thunder_Timer = thunder_Timer - deltaTime
        end

        if thunderReady then
            if thunderTracer_Timer <= 0 then
                thunderTracerParticles:Stop()
                thunderTracer_Timer = thunderTracer_Duration
                strikeThunder(deltaTime)
            else 
                thunderTracer_Timer = thunderTracer_Timer - deltaTime
            end
        end

        if thunderStrikes >= maxThunderStrikes then
            phase1_AttackIdx = 1
            thunderStrikes = 0
        end
    end

end

function rushingAttack(deltaTime)
    if rushWait_Timer <= 0 and readyToRush == false then
        readyToRush = true
        rush_Timer = rush_Duration
        locked_Timer = locked_Duration
        audioSource:PlayWithVariation("event:/Hades_Roar")
    elseif not rushing and readyToRush == false then
        prepareRush(deltaTime)
        rushWait_Timer = rushWait_Timer - deltaTime
    end

    if readyToRush then
        if locked_Timer <= 0 then
            rush(deltaTime)
        else
            locked_Timer = locked_Timer - deltaTime
            if pathMesh:IsVolumetric() then
                if pathMesh.fogColor ~= pathLockedColor then
                    pathMesh.fogColor = pathLockedColor
                    --pathMesh.scattering = oldScatter 
                    --lerpColor = false
                end
            end
        end
    end
end

function resetAfterRush()
    if not initialPos then return end
    readyToRush = false
    rushParticles:Stop()
    --self().transform:SetLocalPosition(initialPos)
    rushWait_Timer = rushWait_Cooldown

    rushing = false

    pathObj:Enable()
    
    incrementPhase2Rush()

    setState(States.Idle)

end

function prepareRush(deltaTime)
    
    setState(States.Idle)
    if not pathObj.enabled then
        pathObj:Enable()
    end

    if pathMesh:IsVolumetric() then
        
            --pathMesh.scattering = pathMesh.scattering + deltaTime
            pathMesh.fogColor = pathSeekColor
        
    end

    local playerPos = player.transform:GetGlobalPosition()
    local position = self().transform:GetGlobalPosition()

    local dir = vec3.new(playerPos.x - position.x, position.y, playerPos.z - position.z)

    rushDirection = normalize(dir.x, dir.y, dir.z)

    rotateTowardsDirection(rushDirection, deltaTime, 10.0)

end

function rush(deltaTime)


    if rush_Timer <= 0 then
        resetAfterRush()
        return
    end

    setState(States.Run)

    rushing = true


    if pathObj.enabled then
        pathObj:Disable()
    end

    rush_Timer = rush_Timer - deltaTime

    rushParticles:Emit()

    local pos = self().transform:GetGlobalPosition()

    local targetPos = pos
    targetPos.x = targetPos.x + rushDirection.x * deltaTime * speed
    targetPos.z = targetPos.z + rushDirection.z * deltaTime * speed

    self().transform:SetLocalPosition(targetPos)
end

function stun()

    audioSource:PlayWithVariation("event:/Hades_OnHit")
    teleportParticles:Emit()
    isStunned = true
    

    setState(States.Stun)

    speakerCounter = speakerCounter + 1
    health = health - 30

    --emit_signal("camera_shake", 10, 2)
    PostProcessing.TriggerCameraShake(0.01, 0.3)
end

function stunned(deltaTime)
    -- play stunned animation
    if stunned_Timer > 0 then
        vulnerable = true
        stunned_Timer = stunned_Timer - deltaTime
    else
        isStunned = false
        vulnerable = false
        stunned_Timer = stunned_Cooldown
        animator:PlayTransition("stun-to-idle")
        resetAfterRush()
        return
    end
end

function prepareThunder(deltaTime)
    local radius = 7

    local randomPos = randomPositionInRadius(player.transform:GetGlobalPosition(), radius)
    randomPos.y = thunder.transform:GetGlobalPosition().y

    thunder.transform:SetLocalPosition(randomPos)
    thunderTracerParticles:Emit()
    thunderReady = true
end

function strikeThunder(deltaTime)
    
    setState(States.Guitar)

    audioSource:PlayWithVariation("event:/Hades_LightingBolt")
    thunderParticles:Emit()
    thunderReady = false
    thunder_Timer = thunder_Cooldown

    thunderStrikes = thunderStrikes + 1
end

--

-- phase 2

local healedPhase2 = false

local healPhase_Duration = 5
local healPhase_Timer = healPhase_Duration

local attack_Cooldown = 0.3
local attack_Timer = attack_Cooldown

local newMaxHealth = 400

local indicatorIdx = 1

local phase2AttackType = 1 -- 1 = dashing, 2 = zigzags, 3 = flee

local preparedPhase2 = false

local maxRushNumber = 3
local rushCounter = 0

local afterRush_Duration = 3
local afterRush_Timer = afterRush_Duration

local maxZigzagNumber = 2
local zigzagCounter = 0

function phase2(deltaTime)
    
    if not preparedPhase2 then
        preparePhase2()
        preparedPhase2 = true
    end

    if not healedPhase2 then
        healPhase2(deltaTime)
        return
    end

    if health < 200 then
        maxRushNumber = 6
        maxZigzagNumber = 4
    end

    if phase2AttackType == 1 then
        
        if rushCounter < maxRushNumber then
            rushingAttack(deltaTime)
            vulnerable = false
        else
            if afterRush_Timer <= 0 then
                rushCounter = 0
                phase2AttackType = 3
                afterRush_Timer = afterRush_Duration
            else 
                vulnerable = true
                setState(States.Stun)
                afterRush_Timer = afterRush_Timer - deltaTime
            end
        end

    elseif phase2AttackType == 2 then
        
        if zigzagCounter < maxZigzagNumber then
            local pos = self().transform:GetGlobalPosition()
            local playerPos = player.transform:GetGlobalPosition()

            local dir = vec3.new(playerPos.x - pos.x, pos.y, playerPos.z - pos.z)

            local rotDir = normalize(dir.x, dir.y, dir.z)  

            rotateTowardsDirection(rotDir, deltaTime, 15)

            vulnerable = false
            if attack_Timer <= 0 then 
                if indicatorIdx <= 4 then
                    zigzag()
                else
                    attackZigzag(deltaTime)
                end
            else
                attack_Timer = attack_Timer - deltaTime
            end
        else 
            zigzagCounter = 0
            phase2AttackType = 1
        end

    elseif phase2AttackType == 3 then
        flee(deltaTime)
    end



end

function incrementPhase2Rush()
    if phaseNumber ~= 2 then return end

    rushCounter = rushCounter + 1
end

function preparePhase2()
    rushWait_Cooldown = 0.8
    rushWait_Timer = rushWait_Cooldown
    speed = 200
end

function healPhase2(deltaTime)
    if not startHealth then 
        startHealth = health
        pathObj:Disable()
        emit_signal("bossHPBar")
    end
    if healPhase_Timer > 0 then
        healPhase_Timer = healPhase_Timer - deltaTime
        local t = math.max(0, math.min(1, (healPhase_Duration - healPhase_Timer) / healPhase_Duration))
        health = mathf.lerp(startHealth, newMaxHealth, t)
    else
        health = newMaxHealth
        healedPhase2 = true
        maxHealth = newMaxHealth
    end
end


function cross(a, b)
    return vec3.new(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    )
end

function subtract(a, b)
    return vec3.new(a.x - b.x, a.y - b.y, a.z - b.z)
end

function add(a, b)
    return vec3.new(a.x + b.x, a.y + b.y, a.z + b.z)
end

function scale(v, s)
    return vec3.new(v.x * s, v.y * s, v.z * s)
end

function distanceXZ(a, b)
    local dx = a.x - b.x
    local dz = a.z - b.z
    return math.sqrt(dx * dx + dz * dz)
end

function randomPointInRadius(radius)
    local angle = math.random() * 2 * math.pi           -- random angle [0, 2π)
    local r = radius * math.sqrt(math.random())         -- sqrt() makes it uniform in circle
    local x = r * math.cos(angle)
    local y = r * math.sin(angle)
    return x, y
end


function zigzag() 
    
    if firePillarParticles.emitting then
        firePillarParticles:Stop()
    end

    local pos = player.transform:GetGlobalPosition()
    pos.y = 0

    if indicatorIdx == 4 then
        PostProcessing.TriggerCameraShake(0.005, 0.3)
    end

    if indicatorIdx < 4 then 
        local x, z = randomPointInRadius(50)
        pos = vec3.new(x, 0, z)
    end

    indicatorsParticles[indicatorIdx]:Emit()

    indicators[indicatorIdx].transform:SetLocalPosition(pos)

    attack_Timer = attack_Cooldown

    indicatorIdx = indicatorIdx + 1
end

function isClose(pos, target)
    local threshold = 0.1  -- how close is to target pos threshold

    local dx = pos.x - target.x
    local dy = pos.y - target.y
    local dz = pos.z - target.z

    local distSq = dx * dx + dy * dy + dz * dz

    return distSq <= threshold * threshold
end

local fireTarget = 1
local fireTargetPos

function attackZigzag(deltaTime)
    if not firePillar then return end

    if not firePillarParticles.emitting then
        --audioSource:PlayWithVariation("event:/Hades_fire")
        firePillarParticles:Emit()
        firePillar:Enable()
    end

    local firePillarPos = firePillar.transform:GetLocalPosition()

    if fireTarget > 4 then
        indicatorIdx = 1
        fireTarget = 1
        for i = 1, 4 do 
            indicatorsParticles[i]:Stop()
        end
        firePillarParticles:Stop()
        firePillar:Disable()
        local p = self().transform:GetGlobalPosition()
        p.y = 0
        firePillar.transform:SetLocalPosition(p)
        zigzagCounter = zigzagCounter + 1
        return
    end

    if not fireTargetPos then
        fireTargetPos = indicators[fireTarget].transform:GetLocalPosition()
    end

    setState(States.Guitar)

    if isClose(firePillarPos, fireTargetPos) then
        fireTarget = fireTarget + 1
        fireTargetPos = nil
        return
    end

    local fireSpeed = 15

    local lerpedX = mathf.lerp(firePillarPos.x, fireTargetPos.x, deltaTime * fireSpeed)
    local lerpedZ = mathf.lerp(firePillarPos.z, fireTargetPos.z, deltaTime * fireSpeed)

    local lerped = vec3.new(lerpedX, 0, lerpedZ)

    firePillar.transform:SetLocalPosition(lerped)
end

local fleeTarget

local maxFleeTime = 3
local fleeTimer = maxFleeTime

function flee(deltaTime)
    if not fleeTarget then
        fleeTarget = findFarthestPoint(fleePoints, player.transform:GetLocalPosition())
    end

    local pos = self().transform:GetLocalPosition()

    if isClose(pos, fleeTarget) then
        phase2AttackType = 2
        fleeTarget = nil
        setState(States.Idle)
        return
    end

    if fleeTimer > 0 then
        fleeTimer = fleeTimer - deltaTime
    else 
        phase2AttackType = 2
        fleeTarget = nil
        setState(States.Idle)
        fleeTimer = maxFleeTime
        return
    end

    local fleeSpeed = 150

    local dir = vec3.new(fleeTarget.x - pos.x, pos.y, fleeTarget.z - pos.z)

    local fleeDirection = normalize(dir.x, dir.y, dir.z)

    rotateTowardsDirection(fleeDirection, deltaTime, 15)

    local targetPos = pos
    targetPos.x = targetPos.x + fleeDirection.x * deltaTime * fleeSpeed
    targetPos.z = targetPos.z + fleeDirection.z * deltaTime * fleeSpeed

    setState(States.Run)
    
    self().transform:SetLocalPosition(targetPos)

end

function findFarthestPoint(points, ref)
    local maxDist2 = -math.huge
    local farthest   = nil       

    for _, p in ipairs(points) do
        local dx = p.x - ref.x
        local dy = p.y - ref.y
        local dz = p.z - ref.z
        local dist2 = dx*dx + dy*dy + dz*dz

        if dist2 > maxDist2 then
            maxDist2 = dist2
            farthest = p
        end
    end
    return farthest
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
    if god then return
    else
        invulTimer = invulDuration
    end
    local attacker = attackerUUID or "Unknown"
    local weapon = hitboxName or "Unknown Weapon"

    audioSource:PlayWithVariation("event:/Hades_OnHit")
    health = health - damage
    if health < 0 then
       health = 0
    end
    
    if health <= 0 then
        die()
    end
end

function die()
    LoadScene("res/scenes/Other/main-menu.scene")
end