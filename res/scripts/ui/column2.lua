local spriteLMB
local spriteRMB

-- Executes when GameObject enters the scene
function onReady()
	connect_signal("column2_update", uuid, function(column, lastInput)
		handleInputChange(column, lastInput)
	end)
end

-- Executes when mouse motion is detected
function onMouseMotion(offsetX, offsetY)
	return
end

-- Executes on each frame
function onUpdate(deltaTime)
	setVariables()
end

function setVariables()
	if not spriteLMB then
		spriteLMB = self():GetChild("LPM")
		if spriteLMB then
			spriteLMB:Disable()
		end
	end
	if not spriteRMB then
		spriteRMB = self():GetChild("PPM")
		if spriteRMB then
			spriteRMB:Disable()
		end
	end
end

function handleInputChange(column, lastInput)
		if lastInput == 0 then
			spriteLMB:Disable()
			spriteRMB:Disable()
		end

		if lastInput == 1 then
			spriteLMB:Enable()
			spriteRMB:Disable()
		end

		if lastInput == 2 then
			spriteLMB:Disable()
			spriteRMB:Enable()
		end
end