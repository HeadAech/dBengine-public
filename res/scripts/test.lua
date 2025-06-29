function onReady()
	return
end

local rotSpeed = 50

export("vec", 1, 2, 3)

function onMouseMotion(offsetX, offsetY)
    --local mousePos = dBengine:GetMousePosition()
    --local cursorLocked = dBengine:IsCursorLocked()
    return
end

function onUpdate(deltaTime)
    --GameObject:Test()
	-- local pos = GameObject:GetLocalPosition()
	-- GameObject:SetLocalPosition(pos.x, pos.y - rotSpeed * deltaTime, pos.z)

    spin(deltaTime)

end

function spin(deltaTime)
    -- start quat

    local currentQuat = self().transform:GetQuatRotation()

    -- rotation delta quaternion
    local angle = rotSpeed * deltaTime
    local s = math.sin(math.rad(angle) * 0.5)
    local c = math.cos(math.rad(angle) * 0.5)

    --local rotDelta = {w = c, x = s, y = s, z = s}
    local rotDelta = quat.new(c, s, s, s)
    
    -- end quat
    local targetQuat = mathf.multiplyQuats(rotDelta, currentQuat)
    -- slerp it
    local smoothQuat = mathf.slerp(currentQuat, targetQuat, 0.1)
    -- Set the new rotation
    self().transform:SetQuatRotation(smoothQuat)
	--local pos = GameObject:GetLocalPosition()
	
end