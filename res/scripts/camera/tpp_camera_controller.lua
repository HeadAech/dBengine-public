local orbitDistance = 15.0
local orbitYaw = 0.0
local orbitPitch = 25.0
local targetHeightOffset = 8.0
local mouseSensitivity = 0.1
local target = nil
local scene = nil
local cameraX = 0
local cameraY = 0  
local cameraZ = 0
local camera

function onReady()
    if not Engine.GetSettings().IsEditorEnabled then
        Input.SetCursorLocked(true)
    end
    scene = GetScene()

    if scene then
        target = scene:GetGameObject("Player")
    end

end
function onUpdate(deltaTime)
    if not camera then
        camera = self():GetComponent("Camera")
    end
    if not target then
        scene = GetScene()
        target = scene:GetGameObject("Player")
    end
    if not camera.isUsed then
        return
    end

    if Input.IsActionJustPressed("toggle_camera") then
        Input.SetCursorLocked(not Input.IsCursorLocked())
    end

    updateCamera()



end

function onMouseMotion(offsetX, offsetY)
    if not (Input.IsCursorLocked() and camera.isUsed) then
        return
    end

    orbitYaw = orbitYaw + (-offsetX * mouseSensitivity)
    orbitPitch = orbitPitch + (-offsetY * mouseSensitivity)

    if orbitPitch > 80 then orbitPitch = 80 end
    if orbitPitch < -22.5 then orbitPitch = -22.5 end

    while orbitYaw > 180 do orbitYaw = orbitYaw - 360 end
    while orbitYaw < -180 do orbitYaw = orbitYaw + 360 end
end

function updateCamera()
    if not target or not target.transform then
        return
    end

    local playerPos = target.transform.globalPosition
    if not playerPos then
        return
    end

    local targetX = playerPos.x
    local targetY = playerPos.y + targetHeightOffset
    local targetZ = playerPos.z

    local radYaw = math.rad(orbitYaw)
    local radPitch = math.rad(orbitPitch)

    local offsetX = orbitDistance * math.cos(radPitch) * math.sin(radYaw)
    local offsetY = orbitDistance * math.sin(radPitch)
    local offsetZ = orbitDistance * math.cos(radPitch) * math.cos(radYaw)

   
    local pos = self().transform.localPosition
    pos.x = targetX + offsetX
    pos.y = targetY + offsetY
    pos.z = targetZ + offsetZ

    self().transform:SetLocalPosition(pos)
  
    local dirX = targetX - pos.x
    local dirY = targetY - pos.y
    local dirZ = targetZ - pos.z
    local dirLength = math.sqrt(dirX*dirX + dirY*dirY + dirZ*dirZ)
    if dirLength > 0.001 then
        dirX = dirX / dirLength
        dirY = dirY / dirLength
        dirZ = dirZ / dirLength
    end
    local yaw = math.deg(mathf.atan2(dirX, dirZ))
    local pitch = math.deg(math.asin(-dirY))
    if pitch > 89 then pitch = 89 end
    if pitch < -89 then pitch = -89 end
    if camera then
        camera.UpdateCamera(yaw, pitch)
    end
end