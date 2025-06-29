export("cutscene", true)

tppCamera = nil
local camera1
local camera2

cutsceneEnd = false

local ui = nil

-- Executes when GameObject enters the scene
function onReady()
	connect_signal("cutscene_ended", uuid, function()
		cutsceneEnd = true
	end)
end

-- Executes on each frame
function onUpdate(deltaTime)
	if not ui then
		local scene = GetScene()
		if scene then
			ui = scene:GetGameObject("UI")
			return
		end
	end


	if not tppCamera then
		local scene = GetScene()
		if scene then
			tppCamera = scene:GetGameObject("TPP Camera")
			return
		end
	end

	if cutsceneEnd then
		cutsceneEnd = false
		if camera2 then
			Engine.UseCamera(camera2:GetUUID(), true)
		end
	end

	if not camera1 then
		camera1 = GetChild("Camera Front")
		return
	end

	if not camera2 then
		camera2 = GetChild("Camera Far")
		return
	end

	if cutscene then
		runCutscene()
		cutscene = false
	end

end

function runCutscene()
	local dialogs = {
		"It's really brave of you, coming here.",
		"... but also really dumb.",
		"Now, I shall show you, where exit is.",
		"Behold..."
	}	
	local events = {
		"idle-to-pose-1",
		"pose-1-to-2",
		"pose-2-to-3",
		"pose-3-to-1"
	}
	local onEnd = "cutscene_ended"

	emit_signal("dialog_set_text", dialogs, events, onEnd)

	ui:Disable()

	Engine.UseCamera(camera1:GetUUID(), true)
end