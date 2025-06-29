local hp = 1.0

local sprite

local ref

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
	
	hp = GetRef().playerHealth

    sprite.clipping.y = mathf.lerp(sprite.clipping.y, 1 - hp, deltaTime * 5)
end

function setVariables()
    if not sprite then
        sprite = self():GetComponent("Sprite")

    end
	if not ref then
		ref = GetRef()

	end
end