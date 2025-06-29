local component_uuid = uuid
local signals = signals
local isLerping = false
local lerpDuration = 0.3
local startIntensity
local endIntensity

local timeElapsed = 0.0
local isForward = true
local setupFlag = true

function onReady()
	return
end

function connectSignals()
    signals.Timer_OnTimeout:connect(component_uuid .. "_onTimeout", function()
        initLerp()
    end, "onBeat")
end

function disconnectSignals()
    signals.Timer_OnTimeout:disconnect(component_uuid .. "_onTimeout")
end

function onUpdate(deltaTime)



    if setupFlag then
        setupFlag = false
        startIntensity = self():GetComponent("PointLight").intensity
        endIntensity = startIntensity + 1.5
    end
    if isLerping then
        timeElapsed = timeElapsed + deltaTime
        local t = timeElapsed / lerpDuration
        if t > 1 then
            t = 1
            if isForward then
                isForward = false
                local temp = startIntensity
                startIntensity = endIntensity
                endIntensity = temp
                timeElapsed = 0.0
            else
                isLerping = false
                timeElapsed = 0.0
            end
        end

        local currentInt = mathf.lerp(startIntensity, endIntensity, t)
        self():GetComponent("PointLight").intensity = currentInt
    end
end

function initLerp()
    if not startIntensity then return end
    if not isLerping then
        isLerping = true
        isForward = true
		startIntensity = self():GetComponent("PointLight").intensity
        endIntensity = startIntensity + 1.5
        timeElapsed = 0.0
    end
end