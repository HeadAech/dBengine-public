

local engineSettings


local og_displacementStrength = 0.004
local og_scanlineProbability = 0.002
local og_scanlineHeight = 3.0

local displacementStrength = 0.05
local scanlineHeight = 5.0
local scanlineProbability = 0.04

local og_glitchEffectIntensity = 0.3
local og_glitchEffectFrequency = 20.0

local glitchEffectIntensity = 1
local glitchEffectFrequency = 60.0

export("deathEffects", true)
local deathEffectsEnabled = false

-- buttons

local quitBtn

local quitPressed = false


-- Executes when GameObject enters the scene
function onReady()
   deathEffects = true
end

-- Executes on each frame
function onUpdate(deltaTime)
	if not engineSettings then
		engineSettings = Engine.GetSettings()
	end

	if not quitBtn then
		local q = self():GetChild("Menu Btn")
		if q then
			quitBtn = q:GetComponent("Button")
		end
	end

	if deathEffects and not deathEffectsEnabled then
		enableDeathEffects()
		deathEffectsEnabled = true
	elseif not deathEffects and deathEffectsEnabled then
		disableDeathEffects()
		deathEffectsEnabled = false
	end

	if quitBtn then
		if quitBtn.pressed and not quitPressed then
			toMainMenu()
			quitPressed = true
		end

		if not quitBtn.pressed and quitPressed then
			quitPressed = false
		end
	end

end

function enableDeathEffects()
	engineSettings.SetGlitchEffectEnabled(true)
	engineSettings.SetDisplacementStrength(displacementStrength)
	engineSettings.SetGlitchEffectIntensity(glitchEffectIntensity)

	engineSettings.SetGlitchEffectFrequency(glitchEffectFrequency)
	engineSettings.SetScanlineHeight(scanlineHeight)
	engineSettings.SetScanlineProbability(scanlineProbability)

	engineSettings:ApplyShaderProperties()

end

function disableDeathEffects()
	engineSettings.SetGlitchEffectEnabled(false)
	engineSettings.SetDisplacementStrength(og_displacementStrength)
	engineSettings.SetGlitchEffectIntensity(og_glitchEffectIntensity)

	engineSettings.SetGlitchEffectFrequency(og_glitchEffectFrequency)
	engineSettings.SetScanlineHeight(og_scanlineHeight)
	engineSettings.SetScanlineProbability(og_scanlineProbability)
	engineSettings.ApplyShaderProperties()
end

function toMainMenu()
	disableDeathEffects()
	LoadScene("res/scenes/Other/main-menu.scene")
end