
-- triggers

local trigger1
local area1

local trigger2
local area2

local trigger3
local area3

local trigger4
local area4

local button1
local button2
local button3
local button4

local firstStart = true

local load_Cooldown = 5
local load_Timer = load_Cooldown

local beginLoad = false

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
    setTriggers()
    setButtons()

    if firstStart then
        firstStart = false
        hideButtons()
    end

    manageEnter()

    if beginLoad then
        if load_Timer <= 0 then
            LoadScene("res/scenes/ArenaLevel.scene")
            return
        else
            load_Timer = load_Timer - deltaTime
        end
    end


end

function setTriggers()
    if trigger1 and trigger2 and trigger3 and trigger4 then
        return
    end

    if not trigger1 then
        trigger1 = GetChild("Trigger1")
        if trigger1 then
            area1 = trigger1:GetComponent("CollisionShape")
        end
    end

    if not trigger2 then
        trigger2 = GetChild("Trigger2")
        if trigger2 then
            area2 = trigger2:GetComponent("CollisionShape")
        end
    end

    if not trigger3 then
        trigger3 = GetChild("Trigger3")
        if trigger3 then
            area3 = trigger3:GetComponent("CollisionShape")
        end
    end

    if not trigger4 then
        trigger4 = GetChild("Trigger4")
        if trigger4 then
            area4 = trigger4:GetComponent("CollisionShape")
        end
    end
end

function setButtons()
    if button1 and button2 and button3 and button4 then
        return
    end

    if not button1 then
        button1 = GetChild("Button1")
    end

    if not button2 then
        button2 = GetChild("Button2")
    end

    if not button3 then
        button3 = GetChild("Button3")
    end

    if not button4 then
        button4 = GetChild("Button4")
    end
end

function hideButtons()
    if button1.enabled then
        button1:Disable()
    end
    if button2.enabled then
        button2:Disable()
    end
    if button3.enabled then
        button3:Disable()
    end
    if button4.enabled then
        button4:Disable()
    end
end

function manageEnter()

    local all = {area1, area2, area3, area4}

    for i, obj in ipairs(all) do

        if obj:IsCollisionArea() then

            local gameObjects = obj.gameObjectsInArea
            for j, go in ipairs(gameObjects) do

                local tag = go:GetComponent("Tag")
                if tag then

                    if tag.name == "Player" then
                        showButton(i)
                    end
                end
            end
        end
    end
end

function showButton(idx)
    local all = {button1, button2, button3, button4}

    if all[idx].enabled then
        return
    end
    hideButtons()
    all[idx]:Enable()
    if idx == 4 then
        beginLoad = true
    end
end