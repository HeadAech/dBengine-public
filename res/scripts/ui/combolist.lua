local sprite
local movingRight = true
local isMoving = false
local startPos = vec2.new(0, 0)
local targetPos = vec2.new(0, 0)
local lerpSpeed = 3.0 -- Możesz dostosować prędkość

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
    setVariables()
    
    if Input.IsActionJustPressed("TAB") and not isMoving then
        startMovement()
    end
    
    if isMoving then
        updateMovement(deltaTime)
    end
end

function setVariables()
    if not sprite then
        sprite = self():GetChild("aaaa"):GetComponent("Sprite")
    end
end

function startMovement()
    if sprite then
        local currentPos = sprite.position
        startPos = vec2.new(currentPos.x, currentPos.y)
        
        if movingRight then
            targetPos = vec2.new(currentPos.x + 300, currentPos.y)
            movingRight = false
        else
            targetPos = vec2.new(currentPos.x - 300, currentPos.y)
            movingRight = true
        end
        
        isMoving = true
    end
end

function updateMovement(deltaTime)
    if sprite then
        local currentPos = sprite.position
        
        -- Lerp pozycji X i Y
        local newX = mathf.lerp(currentPos.x, targetPos.x, deltaTime * lerpSpeed)
        local newY = mathf.lerp(currentPos.y, targetPos.y, deltaTime * lerpSpeed)
        
        sprite.position = vec2.new(newX, newY)
        
        -- Sprawdź czy dotarliśmy do celu (z małą tolerancją)
        local distance = math.abs(currentPos.x - targetPos.x)
        if distance < 1.0 then
            sprite.position = targetPos -- Ustaw dokładną pozycję
            isMoving = false
        end
    end
end