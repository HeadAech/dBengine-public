
local TypeWriter = require("ui.TypeWriter")
local tw

bg = nil
bgSprite = nil

text = nil
textComp = nil

confirm = nil
confirmSprite = nil

first = true

confirm_Cooldown = 1
confirm_Timer = confirm_Cooldown

local canConfirm = false

local visible = true

local currentDialogIdx = 0

-- template
local data = {
    dialogs = {},
    events = {},
    onEndSignal = nil
}

local show = false

function shallowCopy(orig)
    local copy = {}
    for i = 1, #orig do
        table.insert(copy, orig[i])
    end
    return copy
end

-- Executes when GameObject enters the scene
function onReady()
    currentDialogIdx = 0
    connect_signal("dialog_set_text", uuid, function(d, e, onEnd)
        local copy = shallowCopy(d)
        local eventsC = shallowCopy(e)
        local onEndT = onEnd
        test(copy, eventsC, onEndT)
    end)
end

function test(c, e, o)
    data = {
        dialogs = c,
        events = e,
        onEndSignal = o
    }
    showAll()
end

function setVariables()
    if bg and text and confirm and bgSprite and textComp and confirmSprite then
        return true
    end

    if not bg then
        bg = self():GetChild("Bg")
        if bg then
            bgSprite = bg:GetComponent("Sprite")
        end
    end

    if not text then
        text = self():GetChild("Text")
        if text then
            textComp = text:GetComponent("Text")

            tw = TypeWriter:new(
                textComp,
                "Something went wrong! Pls fix!",
                25,
                function()
                    print("Err! Something went wrong with the dialogbox!")
                    canConfirm = true
                end
            )
        end
    end

    if not confirm then
        confirm = self():GetChild("Confirm")
        if confirm then
            confirmSprite = confirm:GetComponent("Sprite")
        end
    end

    return false
end

-- Executes on each frame
function onUpdate(deltaTime)
    if not setVariables() then
        return
    end

    if first then
        hideAll()
        first = false
    end

    if show then
        showAll()
        show = false
    end

    if canConfirm then
        showConfirm(deltaTime)
        if Input.IsActionJustPressed("jump") then
            nextDialog()
        end
    end

    if visible then
        if tw then 
            tw:update(deltaTime)
        end

    end
end

function hideAll()
    currentDialogIdx = 0
    canConfirm = false
    text:Disable()
    bgSprite.modulate.w = 0
    confirmSprite.modulate.w = 0

    visible = false
end

function showAll()
    text:Enable()
    bgSprite.modulate.w = 0.4
    
    confirm_Timer = confirm_Cooldown

    visible = true
    currentDialogIdx = 0
    nextDialog()
end

function showConfirm(deltaTime)
    if confirm_Timer > 0 then
        confirm_Timer = confirm_Timer - deltaTime
        return
    end

    local targetAlpha = 1

    local currentAlpha = confirmSprite.modulate.w

    local lerpedA = mathf.lerp(currentAlpha, targetAlpha, deltaTime * 5)

    confirmSprite.modulate.w = lerpedA

    if not canConfirm and currentAlpha > 0.9 then
        canConfirm = true
    end
end

function nextDialog()
    currentDialogIdx = currentDialogIdx + 1
    canConfirm = false

    if currentDialogIdx > #data.dialogs then
        hideAll()
        if data.onEndSignal then
            emit_signal(data.onEndSignal)
        end
        return
    end

    confirm_Timer = confirm_Cooldown
    confirmSprite.modulate.w = 0

    if tw then
        tw:setText(data.dialogs[currentDialogIdx], 30, function() 
            canConfirm = true
            if data.events[currentDialogIdx] then
                emit_signal(data.events[currentDialogIdx])
            end
        end)
    end
end