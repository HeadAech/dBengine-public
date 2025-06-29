local collisionShapeArea
local particleSystem
local animator = nil
local isComboActive = false
local particleEmitProgress = 0.6
local animationEndProgress = 0.98
local particlesEmitted = false
--Executes when GameObject enters the scene
function onReady()
	connect_signal("combo1_signal", uuid, function()
		attack()
	end)
end
-- Executes on each frame
function onUpdate(deltaTime)
	setVariables()
	handleAnim()
end
function setVariables()
	if not collisionShapeArea then
            collisionShapeArea = self():GetComponent("CollisionShape")		
    end
	if not particleSystem then
		particleSystem = self():GetComponent("ParticleSystem")	
	end
	if not animator then
		animator = self().parent:GetComponent("Animator")
	end
end

function handleAnim()
	if isComboActive and animator then
		local animProgress = animator:GetAnimationProgress()
		if animProgress >= particleEmitProgress and not particlesEmitted then
			particlesEmitted = true
			if particleSystem then
				particleSystem:Emit()
			end
			
			if collisionShapeArea and collisionShapeArea:IsCollisionArea() then
				local gameObjects = collisionShapeArea.gameObjectsInArea
				for i, obj in ipairs(gameObjects) do
					inArea(obj)
				end
			end
		end
		
		if animProgress >= animationEndProgress then
			isComboActive = false
			particlesEmitted = false  
			
			animator:PlayTransition("combo1_to_idle")
			emit_signal("combo_finished")
		end
	end
end

function attack()
	if animator and not isComboActive then
		animator:PlayTransition("idle_to_combo1")
		isComboActive = true
		particlesEmitted = false  
	end
end
function inArea(gameObject)
	local tag = gameObject:GetComponent("Tag")
	local tagN = nil
	if tag then
		tagN = tag.name
	else 
		return
	end
	if tagN == "Enemy" or tagN == "Boss" then
    	emit_signal("combo_damage", 30, "combo1", gameObject.uuid)
	end
end