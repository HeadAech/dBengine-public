local collisionShapeArea
local particleSystem
local animationEndProgress = 0.98


--Executes when GameObject enters the scene
function onReady()

    connect_signal("combo4_signal", uuid, function(damage)
        attack(damage)
    end)
end

-- Executes on each frame
function onUpdate(deltaTime)
    setVariables()

    local temp = self().parent.transform:GetQuatRotation()
    self().transform:SetQuatRotation(temp)
end
function setVariables()
    if not collisionShapeArea then
            collisionShapeArea = self():GetComponent("CollisionShape")
    end
    if not particleSystem then
        particleSystem = self():GetComponent("ParticleSystem")
    end
end
function attack(damage)
	if particleSystem then    
		particleSystem:Emit()
	end

     if collisionShapeArea:IsCollisionArea() then
        local gameObjects = collisionShapeArea.gameObjectsInArea
        for i, obj in ipairs(gameObjects) do
            inArea(obj, damage)
        end
    end
end

function inArea(gameObject, damage)
	local tag = gameObject:GetComponent("Tag")
	local tagN = nil

	if tag then
		tagN = tag.name
	else 
		return
	end

	if tagN == "Enemy" or tagN == "Boss" then
    	emit_signal("combo_damage", math.ceil(damage), "jump combo", gameObject.uuid)
	end
end