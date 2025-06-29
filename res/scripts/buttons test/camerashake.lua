local button

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
	if not button then
		button = self():GetComponent("Button")
	end


    if button.pressed then
		PostProcessing.TriggerCameraShake(0.01, 0.2)
	end
end