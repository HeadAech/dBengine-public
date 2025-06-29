
engineSettings = nil

ref = nil

animateHurt = false
first = true

local og_displacementStrength = 0.004
local og_scanlineProbability = 0.002
local og_scanlineHeight = 3.0

local displacementStrength = 24.7
local scanlineHeight = 8.0
local scanlineProbability = 3.4

local og_glitchEffectIntensity = 0.0
local og_glitchEffectFrequency = 20.0
local og_vignette = 0.4

local vignette = 0.2

local glitchEffectIntensity = 1
local glitchEffectFrequency = 60.0

local reset_Wait = 1
local reset_Timer = reset_Wait

-- Executes when GameObject enters the scene
function onReady()

    connect_signal("player_take_damage", uuid, function(damage)
        animateHurt = true
        first = true
    end)

end


-- Executes on each frame
function onUpdate(deltaTime)
    if not ref then
        ref = GetRef()
        return
    end

    if not engineSettings then
        engineSettings = Engine.GetSettings()
        disableDeathEffects()
        return
    end

    if ref.playerHealth <= 0.5 then
        engineSettings.SetGlitchEffectIntensity(1.0 - ref.playerHealth)
        engineSettings.SetVignetteStrength(0.2)
        engineSettings.ApplyShaderProperties()
    elseif not animateHurt then
        engineSettings.SetGlitchEffectIntensity(0)
        engineSettings.SetVignetteStrength(0.4)
    end

    if animateHurt then
        if first then
            first = false
            setHurtEffects()
            PostProcessing.TriggerCameraShake(0.03, 0.8)
        end
        local lerpSpeed = 5
        local targetDisplacement = mathf.lerp(engineSettings.GetDisplacementStrength(), og_displacementStrength, deltaTime * lerpSpeed)
        local targetGlitchEffectIntensity = mathf.lerp(engineSettings.GetGlitchEffectIntensity(), og_glitchEffectIntensity, deltaTime * lerpSpeed)
        local targetGlitchEffectFrequency = mathf.lerp(engineSettings.GetGlitchEffectFrequency(), og_glitchEffectFrequency, deltaTime * lerpSpeed)
        local targetScanlineHeight = mathf.lerp(engineSettings.GetScanlineHeight(), og_scanlineHeight, deltaTime * lerpSpeed)
        local targetScanlineProbability = mathf.lerp(engineSettings.GetScanlineProbability(), og_scanlineProbability, deltaTime * lerpSpeed)
        local targetVignette = mathf.lerp(engineSettings.GetVignetteStrength(), og_vignette, deltaTime * lerpSpeed)

        apply(targetDisplacement, targetGlitchEffectIntensity, targetGlitchEffectFrequency, 
        targetScanlineHeight, targetScanlineProbability, targetVignette)

        if reset_Timer <= 0 then
            animateHurt = false
            disableDeathEffects()
            reset_Timer = reset_Wait
        else
            reset_Timer = reset_Timer - deltaTime
        end
    end
end

function setHurtEffects()
    engineSettings.SetGlitchEffectEnabled(true)
	engineSettings.SetDisplacementStrength(displacementStrength)
	engineSettings.SetGlitchEffectIntensity(glitchEffectIntensity)

	engineSettings.SetGlitchEffectFrequency(glitchEffectFrequency)
	engineSettings.SetScanlineHeight(scanlineHeight)
	engineSettings.SetScanlineProbability(scanlineProbability)
     engineSettings.SetVignetteStrength(vignette)

	engineSettings:ApplyShaderProperties()
end

function apply(disp, intens, freq, scanH, scanP, vig) 
    engineSettings.SetGlitchEffectEnabled(true)
    engineSettings.SetDisplacementStrength(disp)
	engineSettings.SetGlitchEffectIntensity(intens)

	engineSettings.SetGlitchEffectFrequency(freq)
	engineSettings.SetScanlineHeight(scanH)
	engineSettings.SetScanlineProbability(scanP)
    engineSettings.SetVignetteStrength(vig)

	engineSettings:ApplyShaderProperties()
end

function disableDeathEffects()
	engineSettings.SetDisplacementStrength(og_displacementStrength)
	engineSettings.SetGlitchEffectIntensity(og_glitchEffectIntensity)

	engineSettings.SetGlitchEffectFrequency(og_glitchEffectFrequency)
	engineSettings.SetScanlineHeight(og_scanlineHeight)
	engineSettings.SetScanlineProbability(og_scanlineProbability)
     engineSettings.SetVignetteStrength(og_vignette)
	engineSettings.ApplyShaderProperties()
end