-- Basic Scorched Earth clone

NUM_PLAYERS = 3
G = 0.1

function setup()
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
    current_player = players[1]
end

function setup_bullets()
    bullets = {}
end

function draw()
    handle_input()
    tick_bullets()
    draw_terrain()
    draw_players()
    draw_bullets()
end

function handle_input()
    if keyboard.pressed['left'] then
        current_player.angle = current_player.angle + 1
    end
    if keyboard.pressed['right'] then
        current_player.angle = current_player.angle - 1
    end
    if keyboard.pressed['space'] then
        fire()
    end
end

function fire()
    local player = current_player
    local a = math.rad(player.angle)
    local speed = 10
    local bullet = {
        player = player,
        x = player.x,
        y = player.y,
        vx = math.cos(a)*speed,
        vy = math.sin(a)*speed,
    }
    table.insert(bullets, bullet)
end

function tick_bullets()
    for i, bullet in ipairs(bullets) do
        bullet.x = bullet.x + bullet.vx
        bullet.y = bullet.y + bullet.vy
        bullet.vy = bullet.vy - G
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
    fill(255, 255, 255, 1.0)
    for i, bullet in ipairs(bullets) do
        rect(bullet.x, bullet.y, 1, 1)
    end
end
