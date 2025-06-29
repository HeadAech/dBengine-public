
-- initial variables
local collisionShape
local meshBase

local sparks
local smoke


local speakerAlive = true
local kill = false

local aliveScatter = 2.5
local deadScatter = -5

-- Executes when GameObject enters the scene
function onReady()    
    connect_signal("reset_speaker_state", uuid, function()
        speakerAlive = true
        kill = false
        updateState()
    end)
end

-- Executes when mouse motion is detected
function onMouseMotion(offsetX, offsetY)
	return
end

-- Executes on each frame
function onUpdate(deltaTime)
    if not speakerAlive then return end

    setVariables()

    if collisionShape:IsCollisionArea() then
        local gameObjects = collisionShape.gameObjectsInArea
        for i, obj in ipairs(gameObjects) do
            inArea(obj)
        end
    end

    updateState()
end

function setVariables()
    if not collisionShape then
        collisionShape = self():GetComponent("CollisionShape")
    end

    if not meshBase then
        local meshChild = GetChild("Base")
        if meshChild then
            meshBase = meshChild:GetComponent("MeshInstance")
            --meshBase.density = -0.4
        end
    end

    if not sparks then
        local sparksChild = GetChild("Particles")
        if sparksChild then
            sparks = sparksChild:GetComponent("ParticleSystem")
        end
    end

    if not smoke then
        local smokeChild = GetChild("Smoke Particles")
        if smokeChild then
            smoke = smokeChild:GetComponent("ParticleSystem")
        end
    end
end

function inArea(gameObject)
    local tag = gameObject:GetComponent("Tag")
    if not tag then return end

    if tag.name ~= "Boss" then 
        return   
    end

    -- if boss
    kill = true

    emit_signal("boss_hit_speaker")

end

function updateState()
    if not meshBase then return end

    if kill then 
        speakerAlive = false
        kill = false
    end

    if speakerAlive and meshBase.scattering ~= aliveScatter then
        meshBase.scattering = aliveScatter
        sparks:Stop()
        smoke:Stop()
    end

    if not speakerAlive and meshBase.scattering ~= deadScatter then
        meshBase.scattering = deadScatter
        sparks:Emit()
        smoke:Emit()
    end

    
end