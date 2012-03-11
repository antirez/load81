-- Basic Scorched Earth clone
-- TODO explosion graphics
-- TODO drop players when terrain is destroyed
-- TODO show player numbers

NUM_PLAYERS = 3
G = 0.1
MAX_POWER = 300
STATUS_HEIGHT = 20

function setup()
    ticks = 0
    last_fire_tick = 0
    setup_terrain()
    setup_players()
    setup_bullets()
end

function setup_terrain()
    terrain = {}
    local freqs = {}
    local amplitudes = {}
    local offsets = {}
    local nwaves = 100
    local max_amplitude = 30
    for i = 1, nwaves do
        freqs[i] = math.random()*0.1 + 0.01
        amplitudes[i] = math.random() * (max_amplitude*(i/nwaves))
        offsets[i] = math.random()*WIDTH
    end
    for x = 0, WIDTH-1 do
        local h = HEIGHT/3
        for i = 1, nwaves do
            h = h + math.sin(x*freqs[i] + offsets[i])*amplitudes[i]
        end
        terrain[x] = h
    end
end

function setup_players()
    players = {}
    for i = 1, NUM_PLAYERS do
        local player = {}
        player.x = math.random(10, WIDTH-10)
        player.y = terrain[player.x]
        player.angle = 90
        player.power = 100
        player.r = 255
        player.g = 0
        player.b = 0
        for x = player.x - 8, player.x + 8 do
            if terrain[x] >= player.y then
                terrain[x] = player.y-1
            end
        end
        players[i] = player
    end
    current_player_index = nil
    next_player()
end

function next_player()
    current_player_index = next(players, current_player_index)
    if current_player_index == nil then
        current_player_index = next(players)
    end
    current_player = players[current_player_index]
end

function setup_bullets()
    bullets = {}
    bullets_in_flight = 0
end

function draw()
    ticks = ticks + 1
    handle_input()
    tick_bullets()
    draw_terrain()
    draw_players()
    draw_bullets()
    draw_status()
end

function handle_input()
    if keyboard.pressed['left'] then
        current_player.angle = current_player.angle + 1
    end
    if keyboard.pressed['right'] then
        current_player.angle = current_player.angle - 1
    end
    if keyboard.pressed['up'] and current_player.power < MAX_POWER then
        current_player.power = current_player.power + 1
    end
    if keyboard.pressed['down'] and current_player.power > 0 then
        current_player.power = current_player.power - 1
    end
    if keyboard.pressed['space'] and
       bullets_in_flight == 0 and
       last_fire_tick < ticks - 16 then
        fire()
        last_fire_tick = ticks
    end
end

local next_bullet_id = 1
function fire()
    local player = current_player
    local a = math.rad(player.angle)
    local speed = player.power/10
    local bullet = {
        player = player,
        x = player.x,
        y = player.y,
        vx = math.cos(a)*speed,
        vy = math.sin(a)*speed,
    }
    bullets[next_bullet_id] = bullet
    next_bullet_id = next_bullet_id + 1
    bullets_in_flight = bullets_in_flight + 1
end

function tick_bullets()
    for i, bullet in pairs(bullets) do
        bullet.x = bullet.x + bullet.vx
        bullet.y = bullet.y + bullet.vy
        bullet.vy = bullet.vy - G
        local ix = math.floor(bullet.x+0.5)
        if ix < 0 or ix >= WIDTH then
            bullets[i] = nil
            after_bullet_collision()
        elseif bullet.y < terrain[ix] then
            deform_terrain(ix, math.floor(terrain[ix]), 30)
            bullets[i] = nil
            after_bullet_collision()
        end
    end
end

function deform_terrain(x, y, r)
    for x2 = math.max(x-r,0), math.min(x+r,WIDTH-1) do
        local dx = x2 - x
        local dy = math.sqrt(r*r - dx*dx)
        local ty = terrain[x2]
        local missing = y+dy - ty
        missing = math.max(missing, 0)
        missing = math.min(missing, 2*dy)
        terrain[x2] = ty-2*dy + missing
    end
end

function after_bullet_collision()
    bullets_in_flight = bullets_in_flight - 1
    if bullets_in_flight == 0 then
        next_player()
    end
end

function draw_terrain()
    background(30, 30, 200)
    fill(20, 150, 20, 1.0)
    for i = 0, WIDTH-1 do
        line(i, 0, i, terrain[i])
    end
end

function draw_players()
    for i, player in ipairs(players) do
        fill(player.r, player.g, player.b, 1)
        rect(player.x-6, player.y, 12, 8)
        local l = 13
        line(player.x, player.y+6,
             player.x + l*math.cos(math.rad(player.angle)),
             player.y+6 + l*math.sin(math.rad(player.angle)))
    end
end

function draw_bullets()
    for i, bullet in pairs(bullets) do
        local maxy = HEIGHT-STATUS_HEIGHT
        if (bullet.y < maxy) then
            fill(255, 255, 255, 1.0)
            rect(bullet.x, bullet.y, 1, 1)
        else
            fill(200, 200, 200, 1.0)
            local l = 5
            line(bullet.x-l, maxy-l, bullet.x, maxy)
            line(bullet.x+l, maxy-l, bullet.x, maxy)
        end
    end
end

function draw_status()
    fill(0, 0, 0, 1)
    rect(0, HEIGHT-STATUS_HEIGHT, WIDTH-1, HEIGHT)
    fill(150, 150, 150, 1)
    line(0, HEIGHT-STATUS_HEIGHT-1, WIDTH-1, HEIGHT-STATUS_HEIGHT-1)
    local x = 5
    local padding = 40
    for i, player in ipairs(players) do
        if i == current_player_index then
            fill(255, 255, 255, 1)
        else
            fill(255, 255, 255, 0.5)
        end
        local str = string.format("player %d: angle %d; power %d", i, player.angle, player.power)
        text(x, HEIGHT-18, str)
        x = x + str:len()*10 + padding
        line(x-padding/2, HEIGHT-1, x-padding/2, HEIGHT-STATUS_HEIGHT)
    end
end
