function onReady()
	return
end


export("move", 1)

function onUpdate(deltaTime)

    spin(deltaTime)

end

function spin(deltaTime)
	local pos = GameObject:GetLocalPosition()
    GameObject:SetLocalPosition(pos.x, pos.y + move, pos.z)
	
end