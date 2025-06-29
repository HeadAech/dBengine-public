local hitbox


local canAttack = true

local damage_Cooldown = 1
local damage_Timer = damage_Cooldown

-- Executes when GameObject enters the scene
function onReady()
	return
end


-- Executes on each frame
function onUpdate(deltaTime)
	if not hitbox then
		hitbox = self():GetComponent("CollisionShape")
		return
	end

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
end