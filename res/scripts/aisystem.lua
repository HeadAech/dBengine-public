local aisystem
local anyAgentNeedsPath

-- Executes when GameObject enters the scene
function onReady()
	anyAgentNeedsPath = false
	--connect_signal("enemy_attacked", uuid, function(uid)
	--	ChangeAttacker(uid)
	--end)
end

function setVariables()
    if not aisystem then
            aisystem = self():GetComponent("AISystem")	
    end
end

-- Executes on each frame
function onUpdate(deltaTime)
	setVariables()
	
	if not aisystem:GetTarget() then
        return
    end
   
    local targetPos = aisystem:GetTarget():GetPosition()
    --local agentsInRange = {}
    local agents = aisystem:GetAgents()
	
    for _, agent in pairs(agents) do	
        local agentPos = agent:GetPosition()
        local dist = aisystem:CalculateDistance(agentPos, targetPos)
        local hasLOS = aisystem:HasLineOfSightWithPlayer(agentPos, targetPos)
        local losDistance = agent:GetLineOfSightDistance()
        local stoppingDist = agent:GetStoppingDistance()		

        agent:SetTargetPos(targetPos)
        
        if dist <= stoppingDist + 5.0 then
            --table.insert(agentsInRange, agent)
            
            if dist <= stoppingDist then
                agent:SetState(AIAgentState.ATTACK)
            else 
                agent:SetState(AIAgentState.SEEK)
            end
            
            goto continue
        end

        if not hasLOS and dist >= losDistance then
            agent:SetState(AIAgentState.WANDER)
        elseif hasLOS and dist < losDistance then
            agent:SetState(AIAgentState.SEEK)
        elseif dist < losDistance then
            agent:SetState(AIAgentState.PATH_FOLLOWING)
            anyAgentNeedsPath = true
        end
        
        ::continue::
    end


    if anyAgentNeedsPath then
        aisystem:UpdatePathsBulk()
        anyAgentNeedsPath = false
    end
end

function HandleAttacks(agentsInRange)
    if #agentsInRange == 0 then
        local currentAttacker = aisystem:GetCurrentAttacker()
        if currentAttacker then
            currentAttacker:SetState(AIAgentState.IDLE)
            aisystem:SetCurrentAttacker(nil)
        end
        return
    end
    
    local currentAttacker = aisystem:GetCurrentAttacker()
    local attackerInRange = false

    -- Check if current attacker is still in range
    if currentAttacker then
        for _, agent in pairs(agentsInRange) do
            if agent == currentAttacker then
                attackerInRange = true
                break
            end
        end
    end

    -- If no current attacker or current attacker is not in range, assign new one
    if not currentAttacker or not attackerInRange then
        -- Choose a random agent from agentsInRange
        local randomIndex = math.random(1, #agentsInRange)
        local newAttacker = agentsInRange[randomIndex]
        newAttacker:SetState(AIAgentState.ATTACK)
        aisystem:SetCurrentAttacker(newAttacker)
    end
end

function ChangeAttacker(uid)
    if not aisystem or not uid or aiagent:IsPaused() == true then
        return
    end
    
    -- Find the agent with the matching UUID
    local attackedAgent = nil
    local agents = aisystem:GetAgents()
    

    for _, agent in pairs(agents) do
      
        -- Get the agent's UUID and check if it's valid
        local agentUUID = agent:GetUUID()
        if not agentUUID then
            goto continue
        end
        
        -- Clean both UUIDs by removing non-alphanumeric characters
        local cleanedAgentUUID = agentUUID:gsub("[^%w]", "")
        local cleanedAttackedUUID = uid:gsub("[^%w]", "")
        
        -- Compare the cleaned UUIDs
        if cleanedAgentUUID == cleanedAttackedUUID then
            attackedAgent = agent
            break
        end
        
        ::continue::
    end
    
    if not attackedAgent then
        return
    end
    
    local currentAttacker = aisystem:GetCurrentAttacker()
    -- Set the previous attacker to IDLE if there was one
    if currentAttacker and currentAttacker ~= attackedAgent then
        currentAttacker:SetState(AIAgentState.IDLE)
    end
	
    -- Set the attacked agent as the new attacker
    attackedAgent:SetState(AIAgentState.ATTACK)
    aisystem:SetCurrentAttacker(attackedAgent)
end