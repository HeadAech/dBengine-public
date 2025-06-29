-- REMEMBER!!! values start at 1 in lua instead of 0
-- variables global

lastInputs = {}

resetTimer = 2 -- two seconds

combos = {
	{"lmb", "rmb", "lmb"},
	{"lmb", "lmb", "rmb"},
	{"lmb", "lmb", "lmb"},
	{"rmb", "rmb", "rmb"}
	--{"lmb", "rmb", "rmb"}
}

function onReady()
	return
end

function onMouseMotion(offsetX, offsetY)
	return
end

function onUpdate(deltaTime)
	if Input.IsActionJustPressed("mouse1") then
		addInput("lmb")
		sendChangesToUI()
	end

	if Input.IsActionJustPressed("mouse2") then
		addInput("rmb")
		sendChangesToUI()
	end

	checkCombo()

	expireInputs(deltaTime)
end

function expireInputs(deltaTime)
	if #lastInputs == 0 then return end
	if resetTimer > 0 then
		resetTimer = resetTimer - deltaTime
	else
		lastInputs = {}
		emit_signal("clean_inputqueue", false)
	end

end

function addInput(inp)
	resetTimer = 2
	table.insert(lastInputs, 1, inp)
end

function checkCombo()
	for comboIndex, combo in ipairs(combos) do
		if #lastInputs >= #combo then
			local matched = true

			for i = 1, #combo do
				if lastInputs[i] ~= combo[#combo - i + 1] then
					matched = false
					break
				end
			end

			if matched then
				emit_signal("combo_detected", comboIndex)
				lastInputs = {}
				emit_signal("clean_inputqueue", true)
				resetTimer = 2
				break
			end
		end
	end
end

function connectSignals()
    signals.Timer_OnTimeout:connect(uuid .. "_onTimeout", function()
    end, "onBeat")
end

function disconnectSignals()
    signals.Timer_OnTimeout:disconnect(uuid .. "_onTimeout")
end

function sendChangesToUI()

	local test = lastInputs[1]
	if test == nil then
		emit_signal("input_update", 0)
	end
	if test == "lmb" then
		emit_signal("input_update", 1)
	end
	if test == "rmb" then
		emit_signal("input_update", 2)
	end
end