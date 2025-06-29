local collisionShapeArea
local isActive = false
local validTags = {}
local hitboxName = "Hitbox"
local hitTargetsThisAttack = {} 
local hasBeenActivatedThisAttack = false

function onReady()
    
    connect_signal("hitbox_activate", uuid, onActivate)
    connect_signal("hitbox_deactivate", uuid, onDeactivate)
end

function onUpdate(deltaTime)
    setVariables()
end

function setVariables()
    if not collisionShapeArea then
        collisionShapeArea = self():GetComponent("CollisionShape")
        if collisionShapeArea then
            collisionShapeArea:SetIsCollisionArea(true)
            
        end			
    end
	if #validTags == 0 then
		if self().parent.name == "Player" then
			validTags = {"Enemy", "Boss"}
		elseif self().parent.name == "Enemy" or self().parent.name == "Boss" then
			validTags = {"Player"}
		end
	end
end

function onActivate(targetUUID, damage)
    local cleanTargetUUID = targetUUID:gsub("[^%w]", "")
    local cleanMyUUID = self():GetUUID():gsub("[^%w]", "")
	local cleanParentUUID = self().parent:GetUUID():gsub("[^%w]", "")
	if cleanTargetUUID == cleanMyUUID or cleanTargetUUID == cleanParentUUID then
		if hasBeenActivatedThisAttack then
        	return
    	end
    	hasBeenActivatedThisAttack = true
    	hitTargetsThisAttack = {}
    	isActive = true
    	if not collisionShapeArea or not collisionShapeArea:IsCollisionArea() then
        	return
   	 end
    	local gameObjects = collisionShapeArea.gameObjectsInArea
    	if #gameObjects > 0 then
        	
        	for i, obj in ipairs(gameObjects) do
            	
            	inArea(obj, damage)
        	end
    	end
	end
end

function onDeactivate()
    isActive = false
    hitTargetsThisAttack = {}
    isActive = false
    hasBeenActivatedThisAttack = false
end

function inArea(gameObject, damage)
	hitTargetsThisAttack = {}
	
    if not gameObject then
        return
    end

    local tag = gameObject:GetComponent("Tag")

    if not tag then
        return
    end
    -- chceck if tag valid
    local isValidTarget = false
    for _, validTag in ipairs(validTags) do
        if tag.name == validTag then
            isValidTarget = true
            break
        end
    end
    if not isValidTarget then
        return
    end

    local targetUUID = gameObject:GetUUID()
    
    -- chceck if was hit
    if hitTargetsThisAttack[targetUUID] then
        return
    end
    -- add to hit list
    hitTargetsThisAttack[targetUUID] = true
    
    
	local attackerUUID = self().parent:GetUUID()
    emit_signal("hitbox_hit", damage, hitboxName, attackerUUID, targetUUID)
end