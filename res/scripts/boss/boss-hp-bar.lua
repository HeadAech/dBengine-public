local hp = 1.0
local sprite
local spriteBG
local ref

local shouldGrow = false
local growDuration = 3
local growTimer = growDuration

-- Executes when GameObject enters the scene
function onReady()

    connect_signal("bossHPBar", uuid, function()
        shouldGrow = true
    end)
end

-- Executes when mouse motion is detected
function onMouseMotion(offsetX, offsetY)
	return
end

-- Executes on each frame
function onUpdate(deltaTime)
    setVariables()
    
    hp = ref.bossHealth 

    local totalClip = 1 - hp
    local clipPerSide = totalClip * 0.5
    
    sprite.clipping.x = mathf.lerp(sprite.clipping.x, clipPerSide, deltaTime * 5)
    sprite.clipping.y = mathf.lerp(sprite.clipping.y, clipPerSide, deltaTime * 5)

    if shouldGrow then
        if growTimer > 0 then
            growTimer = growTimer - deltaTime
            updateHealthBar()
        else
            shouldGrow = false
            growTimer = growDuration
        end
    end
end

function setVariables()
    if not sprite then
        sprite = self():GetComponent("Sprite")
    end
    if not ref then
        ref = GetRef()
    end
    if not spriteBG then
        spriteBG = GetScene():GetGameObject("HP Bar bg"):GetComponent("Sprite")
    end
end

function updateHealthBar()
    if not sprite then return end
    sprite.size.x = sprite.size.x + 2

    spriteBG.size.x = spriteBG.size.x + 2
end
