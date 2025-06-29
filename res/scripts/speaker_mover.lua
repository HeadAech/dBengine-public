local component_uuid = uuid
local signals = signals
local isLerping = false
-- get bps / 2.0
local lerpDuration = 0.025
local startScaleVec
local startScale
local endScale

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
        startScaleVec = self().transform.scale
		startScale = startScaleVec.z
        endScale = startScale + 2.0
    end
    if isLerping then
        timeElapsed = timeElapsed + deltaTime
        local t = timeElapsed / lerpDuration
        if t > 1 then
            t = 1
            if isForward then
                isForward = false
                local temp = startScale
                startScale = endScale
                endScale = temp
                timeElapsed = 0.0
            else
                isLerping = false
                timeElapsed = 0.0
            end
        end

        local currentScale = mathf.lerp(startScale, endScale, t)
        local newScale = vec3.new(startScaleVec.x, startScaleVec.y, currentScale)
        self().transform:SetScale(newScale)
    end
end

function initLerp()
    if not startScaleVec then return end
    if not isLerping then
        isLerping = true
        isForward = true
		startScale = startScaleVec.z
        endScale = startScale + 2.0
        timeElapsed = 0.0
    end
end