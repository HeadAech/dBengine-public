local enemies_to_delete = {}
local scene = nil

local waves = {
    {
        enemies = {
            {type = "basic_enemy", amount = 3}
        }
    },
    {
        enemies = {
            {type = "basic_enemy", amount = 4}
        }
    }
}

local current_wave = 1
local enemies_remaining = 0
local total_enemies_in_wave = 0
local defeated_enemies = 0
local enemies_to_spawn = {}

local early_wave_threshold = 0.7  -- 70% of defeated enemies to start next wave
local defeated_enemies_in_current_wave = 0 
local total_enemies_in_current_wave = 0   

local spawn_delay = 1.0  -- 1 sec between spawns
local spawn_timer = spawn_delay
local wave_active = false

local enemy_scene_path = "res/scenes/Objects/enemy2.scene"

local spawn_config = {
    arena_center = vec3.new(0, 0, 0),  
    arena_radius = 45.0,           
    spawn_height = 2.0,           
    min_distance_from_player = 3.0, 
    max_spawn_attempts = 10         
}

function onReady()

    
    connect_signal("enemy_death", uuid, onEnemyDeath)
    scene = GetScene()
   
    
    startWave(current_wave)
end

function onUpdate(deltaTime)
    if #enemies_to_delete > 0 then
        for i, uuid in ipairs(enemies_to_delete) do
            scene:DeleteGameObject(uuid)
        end
        enemies_to_delete = {} 
    end
    
    -- spawn timer
    if wave_active and #enemies_to_spawn > 0 then
        spawn_timer = spawn_timer - deltaTime
        
        if spawn_timer <= 0 then
            spawnNextEnemy()
            spawn_timer = spawn_delay -- Reset timer
        end
    end
end

function randomPositionInRadius(center, radius)
    local angle = math.random() * 2 * math.pi
    local distance = math.sqrt(math.random()) * radius  -- sqrt for uniform distribution
    local height = (math.random() * 2 - 1) * radius      -- random Y offset in [-radius, radius]

    local x = center.x + math.cos(angle) * distance
    local y = center.y + height
    local z = center.z + math.sin(angle) * distance

    return vec3.new(x, y, z)
end

function getRandomSpawnPosition()
    local spawn_pos = randomPositionInRadius(spawn_config.arena_center, spawn_config.arena_radius)
    spawn_pos.y = spawn_config.spawn_height
    return spawn_pos
end

function getPlayerPosition()
    local player = scene:GetGameObject("Player")
    if player then
        return player.transform:GetGlobalPosition()
    end
    return vec3.new(0, 0, 0)  -- if no player
end

function isPositionTooCloseToPlayer(spawn_pos, player_pos)
    local dx = spawn_pos.x - player_pos.x
    local dz = spawn_pos.z - player_pos.z
    local distance = math.sqrt(dx*dx + dz*dz)
    return distance < spawn_config.min_distance_from_player
end

function getValidSpawnPosition()
    local player_pos = getPlayerPosition()
    local attempts = 0
    
    while attempts < spawn_config.max_spawn_attempts do
        local spawn_pos = getRandomSpawnPosition()
        
        if not isPositionTooCloseToPlayer(spawn_pos, player_pos) then
            return spawn_pos
        end
        
        attempts = attempts + 1
    end
    
    local player_to_center = vec3.new(
        spawn_config.arena_center.x - player_pos.x,
        0,
        spawn_config.arena_center.z - player_pos.z
    )
    
    local length = math.sqrt(player_to_center.x * player_to_center.x + player_to_center.z * player_to_center.z)
    if length > 0.001 then
        player_to_center.x = player_to_center.x / length
        player_to_center.z = player_to_center.z / length
    end
    
    return vec3.new(
        spawn_config.arena_center.x + player_to_center.x * spawn_config.arena_radius * 0.8,
        spawn_config.spawn_height,
        spawn_config.arena_center.z + player_to_center.z * spawn_config.arena_radius * 0.8
    )
end

function startWave(wave_number)
    if wave_number > #waves then
        current_wave = 1
        --early_wave_threshold = math.min(early_wave_threshold + 0.05, 0.7)
        wave_number = 1
    end
    local wave = waves[wave_number]
    
    defeated_enemies_in_current_wave = 0
    total_enemies_in_current_wave = 0
    enemies_to_spawn = {}
    
    -- prep enemies for spawn
    for _, enemy_group in ipairs(wave.enemies) do
        for i = 1, enemy_group.amount do
            table.insert(enemies_to_spawn, {
                type = enemy_group.type
            })
            total_enemies_in_current_wave = total_enemies_in_current_wave + 1
        end
    end
    
    enemies_remaining = enemies_remaining + total_enemies_in_current_wave 
    wave_active = true
    spawn_timer = spawn_delay
end

function spawnNextEnemy()
    if #enemies_to_spawn == 0 then
    --all enemies spawned for this wave
        return
    end

    local enemy_data = table.remove(enemies_to_spawn, 1)
    local spawn_position = getValidSpawnPosition()
    local enemy = AddChild(enemy_scene_path)
	
    enemy.transform:SetLocalPosition(spawn_position)
end

function onEnemyDeath(enemyUUID, enemyName, killerUUID)
    defeated_enemies = defeated_enemies + 1
    defeated_enemies_in_current_wave = defeated_enemies_in_current_wave + 1
    enemies_remaining = enemies_remaining - 1

    --treshold check
    if current_wave <= #waves and total_enemies_in_current_wave > 0 then
        local threshold_enemies = math.ceil(total_enemies_in_current_wave * early_wave_threshold)

        if defeated_enemies_in_current_wave >= threshold_enemies and current_wave < #waves then
            current_wave = current_wave + 1
            startWave(current_wave)
        end
    end

    if enemies_remaining <= 0 then
        onAllWavesCleared()
    end
    table.insert(enemies_to_delete, enemyUUID)
end

function onAllWavesCleared()
    
    wave_active = false
    defeated_enemies = 0 
    defeated_enemies_in_current_wave = 0
    -- current_wave = 1
    -- startWave(current_wave) 
    emit("phase_0_finished")
end

function onDestroy()
    disconnect_signal("enemy_death", uuid)
end