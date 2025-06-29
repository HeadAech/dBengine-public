
local spriteL1, spriteL2, spriteL3, spriteL4, spriteL5
local spriteR1, spriteR2, spriteR3, spriteR4, spriteR5
local ogPos

local offsetX = 80.0
local lerpSpeed = 4.0


local inputQueue = {}  
local activeSprites = {} 

local comboSuccessTimer = 0.0
local comboSuccessDuration = 2.5
local wasComboSuccess = false

local isCleaning = false

function onReady()
    connect_signal("input_update", uuid, function(newInput)
        handleInputQueueUpdate(newInput)
    end)

    connect_signal("clean_inputqueue", uuid, function(isCombo)
        if isCombo == true then
            combosuccess()
        else
            cleanInputQueue()
        end
    end)

    setVariables()
    disableAll()
end

function onMouseMotion(offsetX, offsetY)
	return
end

function onUpdate(deltaTime)
    setVariables()
    updateActiveSprites(deltaTime)

    if wasComboSuccess then
        if comboSuccessTimer >= comboSuccessDuration then
            for _, activeSprite in ipairs(activeSprites) do
                if activeSprite.sprite then
                    activeSprite.sprite:GetComponent("Sprite").emission = 1.0
                end
            end
            cleanInputQueue()
            wasComboSuccess = false
        end
        comboSuccessTimer = comboSuccessTimer + deltaTime
    end
end

function setVariables()
    if not spriteL1 then spriteL1 = self():GetChild("L1") if spriteL1 then spriteL1:Disable() end end
    if not spriteL2 then spriteL2 = self():GetChild("L2") if spriteL2 then spriteL2:Disable() end end
    if not spriteL3 then spriteL3 = self():GetChild("L3") if spriteL3 then spriteL3:Disable() end end
    if not spriteL4 then spriteL4 = self():GetChild("L4") if spriteL4 then spriteL4:Disable() end end
    if not spriteL5 then spriteL5 = self():GetChild("L5") if spriteL5 then spriteL5:Disable() end end

    if not spriteR1 then spriteR1 = self():GetChild("R1") if spriteR1 then spriteR1:Disable() end end
    if not spriteR2 then spriteR2 = self():GetChild("R2") if spriteR2 then spriteR2:Disable() end end
    if not spriteR3 then spriteR3 = self():GetChild("R3") if spriteR3 then spriteR3:Disable() end end
    if not spriteR4 then spriteR4 = self():GetChild("R4") if spriteR4 then spriteR4:Disable() end end
    if not spriteR5 then spriteR5 = self():GetChild("R5") if spriteR5 then spriteR5:Disable() end end

    if not ogPos and spriteL1 then
        local currentPos = spriteL1:GetComponent("Sprite").position
        ogPos = vec2.new(currentPos.x, currentPos.y)
    end
end

function getAvailableSprite(inputType)
    local availableSprites = {}
    
    if inputType == 1 then
        availableSprites = {spriteL1, spriteL2, spriteL3, spriteL4, spriteL5}
    elseif inputType == 2 then
        availableSprites = {spriteR1, spriteR2, spriteR3, spriteR4, spriteR5}
    end
    

    for _, sprite in ipairs(availableSprites) do
        if sprite then
            local isUsed = false
            for _, activeSprite in ipairs(activeSprites) do
                if activeSprite.sprite == sprite then
                    isUsed = true
                    break
                end
            end
            if not isUsed then
                return sprite
            end
        end
    end
    
    return nil
end

function handleInputQueueUpdate(newInput)
    if wasComboSuccess then
        return
    end
    
    table.insert(inputQueue, 1, newInput)
    
    if #inputQueue > 3 then
        table.remove(inputQueue, 4)
    end
    
    for _, activeSprite in ipairs(activeSprites) do
        activeSprite.queueIndex = activeSprite.queueIndex + 1
    end
    
    for _, activeSprite in ipairs(activeSprites) do
        if activeSprite.queueIndex > 3 then
            activeSprite.isExiting = true
            activeSprite.fadeOut = true 
        end
    end
    
    local newSprite = getAvailableSprite(newInput)
    if newSprite and ogPos then
        newSprite:Enable()
        
        local spriteComponent = newSprite:GetComponent("Sprite")
        if spriteComponent then
            spriteComponent.position = vec2.new(ogPos.x, ogPos.y)
            spriteComponent.modulate = vec4.new(1, 1, 1, 0) 
        end
        
        table.insert(activeSprites, {
            sprite = newSprite,
            input = newInput,
            queueIndex = 1,
            isExiting = false,
            fadeIn = true,
            fadeOut = false,
            alpha = 0.0 
        })
        
    end
    
    updateTargetPositions()

end

function updateTargetPositions()
    if not ogPos then return end
    
    for _, activeSprite in ipairs(activeSprites) do
        local targetX = ogPos.x + offsetX * (activeSprite.queueIndex - 1)
        local targetY = ogPos.y
        
        if activeSprite.isExiting then
            targetX = ogPos.x + offsetX * 4 
        end
        
        activeSprite.targetPos = {x = targetX, y = targetY}
    end
end

function updateActiveSprites(deltaTime)
    if not ogPos then return end
    
    local spritesToRemove = {}
    local fadeSpeed = 9.0 
    
    for i, activeSprite in ipairs(activeSprites) do
        if activeSprite.sprite and activeSprite.targetPos then
            local spriteComponent = activeSprite.sprite:GetComponent("Sprite")
            if spriteComponent then
                local currentPos = spriteComponent.position
                local targetX = activeSprite.targetPos.x
                local targetY = activeSprite.targetPos.y
                
                local newX = mathf.lerp(currentPos.x, targetX, deltaTime * lerpSpeed)
                local newY = mathf.lerp(currentPos.y, targetY, deltaTime * lerpSpeed)
                spriteComponent.position = vec2.new(newX, newY)
                
                if activeSprite.fadeIn then
                    activeSprite.alpha = activeSprite.alpha + deltaTime * fadeSpeed
                    if activeSprite.alpha >= 1.0 then
                        activeSprite.alpha = 1.0
                        activeSprite.fadeIn = false
                    end
                end
                
                if activeSprite.fadeOut then
                    activeSprite.alpha = activeSprite.alpha - deltaTime * fadeSpeed
                    if activeSprite.alpha <= 0.0 then
                        activeSprite.alpha = 0.0
                    end
                end
                
                local currentModulate = spriteComponent.modulate
                spriteComponent.modulate = vec4.new(currentModulate.x, currentModulate.y, currentModulate.z, activeSprite.alpha)
                
                if isCleaning and activeSprite.alpha <= 0.0 then
                    table.insert(spritesToRemove, i)

                elseif activeSprite.isExiting then
                    local distanceToTarget = math.abs(newX - targetX)
                    if distanceToTarget < 5.0 or activeSprite.alpha <= 0.0 then
                        table.insert(spritesToRemove, i)

                    end
                elseif activeSprite.alpha <= 0.0 and activeSprite.fadeOut then
                    table.insert(spritesToRemove, i)

                end
            end
        end
    end
    
    for i = #spritesToRemove, 1, -1 do
        local index = spritesToRemove[i]
        local activeSprite = activeSprites[index]
        
        if activeSprite and activeSprite.sprite then
            activeSprite.sprite:Disable()
            resetSpritePosition(activeSprite.sprite)

        end
        
        table.remove(activeSprites, index)
    end
    
    -- Check if sprite cleanup is finished, if yes, finalize it.
    if isCleaning and #activeSprites == 0 then
        finalizeCleanup()
    end
end

function resetSpritePosition(sprite)
    if sprite and ogPos then
        local comp = sprite:GetComponent("Sprite")
        if comp then
            comp.position = vec2.new(ogPos.x, ogPos.y)
            comp.emission = 1.0
            comp.modulate.w = 0.0 
        end
    end
end

function cleanInputQueue()

    isCleaning = true
    
    for _, activeSprite in ipairs(activeSprites) do
        if activeSprite.sprite then
            activeSprite.fadeOut = true
            activeSprite.fadeIn = false
        end
    end
    
    if #activeSprites == 0 then
        finalizeCleanup()
    end
end

function finalizeCleanup()

    
    for _, activeSprite in ipairs(activeSprites) do
        if activeSprite.sprite then
            activeSprite.sprite:Disable()
            resetSpritePosition(activeSprite.sprite)
        end
    end
    
    inputQueue = {}
    activeSprites = {}
    isCleaning = false
    
end

function combosuccess()
    comboSuccessTimer = 0.0
    wasComboSuccess = true

    for _, activeSprite in ipairs(activeSprites) do
        if activeSprite.sprite then
            activeSprite.sprite:GetComponent("Sprite").emission = 2.0
        end
    end
end

function disableAll()

   
   local sprites = {spriteL1, spriteL2, spriteL3, spriteL4, spriteL5, spriteR1, spriteR2, spriteR3, spriteR4, spriteR5}
   
   for i, sprite in ipairs(sprites) do
       if sprite then
           sprite:Disable()
           resetSpritePosition(sprite)

       end
   end
   
   inputQueue = {}
   activeSprites = {}
   isCleaning = false
   wasComboSuccess = false
   comboSuccessTimer = 0.0
   
end