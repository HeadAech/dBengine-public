-- variables

local enemies = {}

local target

export("travelSpeed", 3)

function onReady()
	return
end

function onUpdate(deltaTime)
	if #enemies ~= GetChildrenCount() then
		local children = GetChildren()
		enemies = children
	end
    
	if not target then
		local scene = GetScene()
		target = scene:GetGameObject("Cube")
	end

	if Input:IsActionJustPressed("jump") then
		spawnEnemy()
	end
	
    for i, child in ipairs(enemies) do
        moveToTarget(child, deltaTime)
    end

    changePitch(#enemies)
end

function spawnEnemy()
	local pathToScene = "res/scenes/enemy.scene"

	local e = AddChild(pathToScene)

	table.insert(enemies, e)

end

function moveToTarget(object, deltaTime)
    if not target then
        return
    end

    local objTransform = object.transform
    local targetTransform = target.transform

    local objPos = objTransform:GetGlobalPosition()
    local targetPos = targetTransform:GetGlobalPosition()

    -- directional vector
    local dx = targetPos.x - objPos.x
    local dy = targetPos.y - objPos.y
    local dz = targetPos.z - objPos.z

    local distance = math.sqrt(dx*dx + dy*dy + dz*dz)
    if distance < 0.01 then return end

    -- Normalize direction
    dx = dx / distance
    dy = dy / distance
    dz = dz / distance

    -- Move towards target
    objPos.x = objPos.x + dx * travelSpeed * deltaTime
    objPos.y = objPos.y + dy * travelSpeed * deltaTime
    objPos.z = objPos.z + dz * travelSpeed * deltaTime

    objTransform:SetLocalPosition(objPos)
end

function changePitch(number)
	 
    --local src = GetScene():GetGameObject("Audio Source")

    --local comp = src:GetComponent("AudioSource")
    --comp:SetPitch("event:/Test Music", 1.0 + number * 0.1)

end