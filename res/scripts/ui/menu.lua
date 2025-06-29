local startBtn = nil
local startPressed = false

local quitBtn = nil
local quitPressed = false


-- Executes when GameObject enters the scene
function onReady()
	Input.SetCursorLocked(false)
end


-- Executes on each frame
function onUpdate(deltaTime)
	if not startBtn then
		local s = GetChild("Start Button")
		if s then
			startBtn = s:GetComponent("Button")
		end
		return
	end

	if not quitBtn then
		local s = GetChild("Quit Button")
		if s then
			quitBtn = s:GetComponent("Button")
		end
		return
	end

	if startBtn.pressed and not startPressed then
		startPressed = true
		start()
	end

	if not startBtn.pressed and startPressed then
		startPressed = false
	end

	if quitBtn.pressed and not quitPressed then
		quitPressed = true
		quit()
	end

	if not quitBtn.pressed and quitPressed then
		quitPressed = false
	end

end

function start()
	LoadScene("res/scenes/ArenaLevel2.scene")
end

function quit()
	Engine.Quit()
end