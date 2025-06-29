-- Executes when GameObject enters the scene
function onReady()
	return
end

-- Executes when mouse motion is detected
function onMouseMotion(offsetX, offsetY)
	return
end

-- Executes on each frame
function onUpdate(deltaTime)
	local pos_x = GameObject:GetLocalPosition().x
    TextRenderer:SetText(tostring(pos_x))
end