
local boss = nil

local animator = nil
tppCamera = nil

States = {
    Idle = 0,
    Pose1 = 1,
    Pose2 = 2,
    Pose3 = 3,
    Jump = 4,
    Scream = 5
}
state = States.Idle

stateToSet = nil

local initPos = vec3.new(-79.659683, 21.300640, 31.045004)

local jumpPos = vec3.new(-72.702904, 11.597001, 31.323402)
local screamPos = vec3.new(-62.761093, 11.597001, 27.317509)

local jump_Timer = 3.7

local scream_Timer = 2.7

local ui = nil

-- Executes when GameObject enters the scene
function onReady()
    self():Enable()
    self().transform:SetLocalPosition(initPos)
    connect_signal("idle-to-pose-1", uuid, function()
        stateToSet = States.Pose1
    end)

    connect_signal("pose-1-to-2", uuid, function()
        stateToSet = States.Pose2
    end)

    connect_signal("pose-2-to-3", uuid, function()
        stateToSet = States.Pose3
    end)

    connect_signal("pose-3-to-1", uuid, function()
        stateToSet = States.Pose1
    end)

    connect_signal("cutscene_ended", uuid, function()
        stateToSet = States.Jump
    end)
end

local screamed = false
local audioSource

-- Executes on each frame
function onUpdate(deltaTime)
    if not audioSource then
        audioSource = self():GetComponent("AudioSource")
    end

    if not ui then
        local scene = GetScene()
		if scene then
			ui = scene:GetGameObject("UI")
			return
		end
    end

    if not tppCamera then
		local scene = GetScene()
		if scene then
			tppCamera = scene:GetGameObject("TPP Camera")
			return
		end
	end

    if not boss then
        local scene = GetScene()
		if scene then
			boss = scene:GetGameObject("Boss")
            boss:Disable()
			return
		end
    end

    if not animator then
        animator = self():GetComponent("Animator")
        if animator then
            animator:PlayTransition("idle-to-vibe")
        end
        return
    end

    if stateToSet then
        setState(stateToSet)
        stateToSet = nil
    end

    if state == States.Jump then
        if jump_Timer <= 0 then
            setState(States.Scream)
            self().transform:SetLocalPosition(screamPos)
        elseif jump_Timer <= 2.3 then

            local pos = self().transform:GetGlobalPosition()

            local lerpedX = mathf.lerp(pos.x, jumpPos.x, deltaTime * 7)
            local lerpedY = mathf.lerp(pos.y, jumpPos.y, deltaTime * 7)
            local lerpedZ = mathf.lerp(pos.z, jumpPos.z, deltaTime * 7)

            local lerped = vec3.new(lerpedX, lerpedY, lerpedZ)

            self().transform:SetLocalPosition(lerped)

            jump_Timer = jump_Timer - deltaTime
        else
            jump_Timer = jump_Timer - deltaTime
        end
    end

    if state == States.Scream then
        if scream_Timer <= 0 then
            if tppCamera then
                Engine.UseCamera(tppCamera:GetUUID(), false)
            end
            boss:Enable()
            ui:Enable()
            self():Disable()
        else
            if scream_Timer < 1.7 and not screamed then
                screamed = true
                --audioSource:PlayWithVariation("event:/Hades_Howl")
                PostProcessing.TriggerCameraShake(0.05, 1.5)
            end
            scream_Timer = scream_Timer - deltaTime
        end
    end

end

function setState(s)
    if state == s then return end

    local oldState = state

    state = s

    if oldState == States.Idle and state == States.Pose1 then
        animator:PlayTransition("idle-to-pose-1")
        return
    end

    if oldState == States.Pose1 and state == States.Pose2 then
        animator:PlayTransition("pose-1-to-2")
        return
    end

    if oldState == States.Pose2 and state == States.Pose3 then
        animator:PlayTransition("pose-2-to-3")
        return
    end

    if oldState == States.Pose3 and state == States.Pose1 then
        animator:PlayTransition("pose-3-to-1")
        return
    end

    if state == States.Jump then
        animator:PlayTransition("idle-to-jump")
        return
    end

    if state == States.Scream then
        animator:PlayTransition("jump-to-scream")
        return
    end
    

end