local text

local hide_Cooldown = 3
local hide_Timer = 0

-- Executes when GameObject enters the scene
function onReady()
     connect_signal("set_subtitle", uuid, function(subtitle)
        setText(subtitle)
        end)

    connect_signal("hide_subtitles", uuid, function()
        if text then
            text:SetText("")
        end
    end)
end

-- Executes when mouse motion is detected
function onMouseMotion(offsetX, offsetY)
	return
end

-- Executes on each frame
function onUpdate(deltaTime)
    if not text then
        text = self():GetComponent("Text")
        
    end

end

function setText(txt)
    if text then
        local subLen = #txt
        text.size.x = subLen * 7.4
        text:SetText(txt)
    end
end