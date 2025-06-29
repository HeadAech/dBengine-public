local speed = 10
local normalSpeed = 10
local fastSpeed = 40

local camera

function onReady()
	return
end

function onMouseMotion(offsetX, offsetY)
    if not (Input.IsCursorLocked() and camera.isUsed) then
        return
    end

    local mouseSensitivity = 0.2
 
    local yawDelta = -offsetX * mouseSensitivity

    local pitchDelta = -offsetY * mouseSensitivity 

    -- Get current Yaw and Pitch
    local currentYaw = camera.yaw
    local currentPitch = camera.pitch
    -- New Yaw and Pitch
    local newYaw = currentYaw + yawDelta
    local newPitch = currentPitch + pitchDelta

    -- Clamp Pitch to [-89, 89] degrees [weren't there any clamp functions tho?]
    if newPitch > 89.0 then
        newPitch = 89.0
    elseif newPitch < -89.0 then
        newPitch = -89.0
    end

    -- Update camera with new Yaw and Pitch
    --Camera:UpdateCamera(newYaw, newPitch)
    if camera then
        camera.UpdateCamera(newYaw, newPitch)
    end
end

function onUpdate(deltaTime)
    if not camera then
        camera = self():GetComponent("Camera")
        return
    end
    if not camera.isUsed then
        return
    end

    if Input.IsActionJustPressed("toggle_camera") then
        Input.SetCursorLocked(not Input.IsCursorLocked())
    end

    if Input.IsActionPressed("forward") then
        movement(deltaTime, "forward")
    end

    if Input.IsActionPressed("backward") then
        movement(deltaTime, "backward")
    end

    if Input.IsActionPressed("left") then
        movement(deltaTime, "left")
    end

    if Input.IsActionPressed("right") then
        movement(deltaTime, "right")
    end

end

function movement(deltaTime, direction)
    if not (Input.IsCursorLocked() and camera.isUsed) then
        return
    end

    if Input.IsActionPressed("left_shift") then
        speed = fastSpeed
    else
        speed = normalSpeed
    end

    local front = camera.front
    local right = camera.right
    local pos = self().transform.localPosition
    local velocity = deltaTime * speed
    local dir = 1

    if direction == "forward" or direction == "backward" then
        if direction == "backward" then
            dir = -1
        end
        pos.x = pos.x + front.x * dir * velocity
        pos.y = pos.y + front.y * dir * velocity
        pos.z = pos.z + front.z * dir * velocity
    end

    if direction == "left" or direction == "right" then
        if direction == "left" then
            dir = -1
        end
        pos.x = pos.x + right.x * dir * velocity
        pos.y = pos.y + right.y * dir * velocity
        pos.z = pos.z + right.z * dir * velocity
    end

    self().transform:SetLocalPosition(pos)
end
