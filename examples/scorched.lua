-- Basic Scorched Earth clone.
-- Copyright (C) 2012 Rich Lane
-- This code is released under the BSD two-clause license.
--
-- Use the arrow keys to adjust the angle and power of the tank gun. Hit
-- space to fire. The last tank alive wins!
--
-- TODO Collide bullets directly with players.
-- TODO Fall damage.

NUM_PLAYERS = 3
G = 0.1 -- Acceleration due to gravity (pixels/frame).
MAX_POWER = 300
STATUS_HEIGHT = 75
MAX_WIND = 2
BULLET_DRAG = 0.002

function setup()
    ticks = 0
    last_fire_tick = 0
    setup_terrain()
    setup_players()
    setup_bullets()
    setup_explosions()
    setup_wind()
end

-- Generate terrain using the midpoint displacement algorithm.
function setup_terrain()
    -- Tunables.
    local initial_height = 200
    local max_displacement = 100
    local displacement_growth = 0.92
    local iterations = 1000

    -- x0 and width must be integers.
    local lines = { { x0=0, width=400, slope=0.2 }, { x0=400, width=400, slope=-0.2 } }

    -- For each iteration, pick a random line and split it in two. Move the
    -- middle point up or down by a random amount. The maximum amount it can
    -- be moved decreases with each iteration.
    local y0 = initial_height
    for i = 1, iterations do
        local j = math.random(1, #lines)
        local lA = lines[j]
        local y1 = y0 + lA.width*lA.slope
        local widthA = math.ceil(lA.width/2)
        local widthB = math.floor(lA.width/2)
        if widthB == 0 then
            -- skip
        else
            local midy = y0+lA.width*lA.slope/2
            local dy = (math.random()*2-1)*max_displacement
            -- y = y0 + slope*x
            -- slope = (y-y0)/x
            local slopeA = (midy+dy-y0)/widthA
            local slopeB = (y1-midy-dy)/widthB
            local lB = { x0=lA.x0+widthA, width=widthB, slope=slopeB }
            table.insert(lines, lB)
            lA.width = widthA
            lA.slope = slopeA
            max_displacement = max_displacement * displacement_growth
        end
        y0 = y1
    end

    -- Sort the lines by x0.
    table.sort(lines, function(a,b) return a.x0 < b.x0 end)

    -- Build the terrain height-map from the lines.
    terrain = {}
    local y0 = initial_height
    for i = 1, #lines do
        local l = lines[i]
        for x = l.x0, (l.x0+l.width-1) do
            terrain[x] = y0 + (x-l.x0)*l.slope
        end
        y0 = y0 + l.width*l.slope
    end
end

function setup_players()
    players = {}
    live_players = NUM_PLAYERS
    for i = 1, NUM_PLAYERS do
        local player = {}
        player.x = math.random(10, WIDTH-10)
        player.y = terrain[player.x]
        player.angle = 90
        player.power = 100
        player.health = 100
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

-- Pick a new non-dead player
function next_player()
    if live_players == 0 then return end
    current_player_index = next(players, current_player_index)
    current_player = players[current_player_index]
    if current_player_index == nil or current_player.health == 0 then
        return next_player()
    end
end

-- Choose a color representing the player's status (current, dead, alive)
function player_status_fill(player)
    if player == current_player then
        fill(220, 220, 220, 1)
    elseif player.health == 0 then
        fill(100, 0, 0, 1)
    else
        fill(127, 127, 127, 1)
    end
end

function setup_bullets()
    bullets = {}
    bullets_in_flight = 0
end

function setup_explosions()
    explosions = {}
end

function setup_wind()
    wind = (math.random()*2-1)*MAX_WIND
end

-- If there is only one live player left return its index. Otherwise return nil.
function find_victor()
    local winner_index = nil
    for i, player in ipairs(players) do
        if player.health > 0 then
            if winner_index then
                return
            else
                winner_index = i
            end
        end
    end
    return winner_index
end

function draw()
    ticks = ticks + 1
    local game_over = live_players <= 1
    if not game_over then
        handle_input()
        tick_bullets()
    end
        
    draw_sky()
    draw_terrain()
    draw_players()
    draw_bullets()
    draw_explosions()
    draw_status()
    draw_wind()

    if game_over then
        fill(230, 230, 230, 1)
        if live_players == 1 then
            local winning_player_index = find_victor()
            local str = string.format("Player %d wins!", winning_player_index)
            text(WIDTH/2-str:len()*5, HEIGHT/2, str)
        else
            text(WIDTH/2-5*5, HEIGHT/2, "Draw!")
        end
    end

    if ticks < 30*3 then
        local function centerText(y, str) text(WIDTH/2-str:len()*5, y, str) end
        fill(200, 200, 200, 1-ticks/90)
        centerText(460, "Use the arrow keys to control your tank's gun.")
        centerText(445, "Press the space key to fire.")
    end
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
    if keyboard.pressed['return'] and
       bullets_in_flight == 0 and
       last_fire_tick < ticks - 16 then
        fire_secret()
        last_fire_tick = ticks
    end
end

local next_bullet_id = 1
function add_bullet(bullet)
    bullets[next_bullet_id] = bullet
    next_bullet_id = next_bullet_id + 1
    bullets_in_flight = bullets_in_flight + 1
end

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
        exp_radius = 30,
        exp_damage = 50,
    }
    add_bullet(bullet)
end

function fire_secret()
    local player = current_player
    local speed = player.power/10
    for i = 1,10 do
        local a = math.rad(player.angle) + math.random()*0.05-0.1
        local bullet = {
            player = player,
            x = player.x,
            y = player.y,
            vx = math.cos(a)*speed,
            vy = math.sin(a)*speed,
            exp_radius = 10,
            exp_damage = 40,
        }
        add_bullet(bullet)
    end
end

-- Drag increases with the square of the difference in velocity between
-- the object and the air. Returns the acceleration due to drag.
function bullet_drag(v)
    local a = v^2 * BULLET_DRAG
    if v > 0 then a = -a end -- Oppose velocity.
    return a
end

-- Do bullet physics and check for collisions with terrain or the sides of the screen.
function tick_bullets()
    for i, bullet in pairs(bullets) do
        bullet.x = bullet.x + bullet.vx
        bullet.y = bullet.y + bullet.vy
        bullet.vx = bullet.vx + bullet_drag(bullet.vx-wind)
        bullet.vy = bullet.vy + bullet_drag(bullet.vy) - G
        local ix = math.floor(bullet.x+0.5)
        if ix < 0 or ix >= WIDTH then
            bullets[i] = nil
            after_bullet_collision()
        elseif bullet.y < terrain[ix] then
            bullet.y = terrain[ix]
            explosions[i] = { x=bullet.x, y=bullet.y, r=bullet.exp_radius, ttl=20, lifetime=20 }
            damage_players(bullet.x, bullet.y, bullet.exp_radius, bullet.exp_damage)
            deform_terrain(ix, bullet.y, bullet.exp_radius)
            bullets[i] = nil
            after_bullet_collision()
        end
    end
end

-- Find the players in the damage radius of an explosion and reduce their health.
function damage_players(x, y, r, s)
    for i, player in ipairs(players) do
        local d = math.sqrt((player.x-x)^2 + (player.y-y)^2)
        if player.health > 0 and d < r then
            -- Damage attenuates linearly with distance (in 2d).
            local e = s*(1-(d/r))
            player.health = math.max(player.health - e, 0)
            if player.health == 0 then
                live_players = live_players - 1
            end
        end
    end
end

-- Remove a circular chunk of terrain.
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

    -- Drop players
    for i, player in ipairs(players) do
        local ty = terrain[math.floor(player.x)]
        if player.y > ty+1 then
            player.y = ty+1
        end
    end
end

-- If all the bullets have collided switch to the next player.
function after_bullet_collision()
    bullets_in_flight = bullets_in_flight - 1
    if bullets_in_flight == 0 then
        next_player()
    end
end

function draw_sky()
    local top = HEIGHT-STATUS_HEIGHT-1
    local alpha = bullets_in_flight > 0 and 0.5 or 1
    for i = 0, top do
        local f = (1-(i/top))
        fill(30*f, 30*f, 200*f, alpha)
        line(0, i, WIDTH-1, i)
    end
end

function draw_terrain()
    for i = 0, WIDTH-1 do
        local h = terrain[i]

        -- Draw a vertical line from the bottom
        fill(36, 142, 36, 1.0)
        line(i, 0, i, math.floor(h))

        -- Antialias by drawing a point with transparency equal to the
        -- fraction of a pixel that was ignored above.
        local frac = h-math.floor(h)
        if frac > 0 then
            fill(36, 142, 36, frac)
            rect(i, math.ceil(h), 1, 1)
        end
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
        player_status_fill(player)
        text(player.x-5, player.y-18, tostring(i))
    end
end

function draw_bullets()
    for i, bullet in pairs(bullets) do
        local maxy = HEIGHT-STATUS_HEIGHT
        if (bullet.y < maxy) then
            fill(200, 200, 200, 1.0)
            rect(bullet.x, bullet.y, 2, 2)
        else
            fill(200, 200, 200, 1.0)
            local l = 5
            line(bullet.x-l, maxy-l, bullet.x, maxy)
            line(bullet.x+l, maxy-l, bullet.x, maxy)
        end
    end
end

function draw_explosions()
    for i, exp in pairs(explosions) do
        fill(251, 130, 48, 1)
        local r = exp.r * exp.ttl/exp.lifetime
        ellipse(exp.x, exp.y, r, r)
        exp.ttl = exp.ttl - 1
        if exp.ttl == 0 then explosions[i] = nil end
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
        player_status_fill(player)
        local dy = 17
        text(x, HEIGHT-1*dy, ("player %d"):format(i))
        text(x, HEIGHT-2*dy, ("health: %d"):format(player.health))
        text(x, HEIGHT-3*dy, ("angle: %d"):format(player.angle))
        text(x, HEIGHT-4*dy, ("power: %d"):format(player.power))
        x = x + 15*10 + padding
        fill(150, 150, 150, 1)
        line(x-padding/2, HEIGHT-1, x-padding/2, HEIGHT-STATUS_HEIGHT)
    end
end

function draw_wind()
    fill(79, 64, 19, 1)
    local scale = 100
    local cx = WIDTH/2
    local wx = cx + wind*scale/MAX_WIND
    local wy = 30
    text(cx-20, wy-20, "wind")
    line(cx, wy+3, cx, wy-3)
    local dir = wind > 0 and 1 or -1
    local wsx = cx+scale*dir
    line(wsx, wy+3, wsx, wy-3)
    line(cx, wy, wx, wy)
    line(wx, wy, wx-3*dir, wy-3)
    line(wx, wy, wx-3*dir, wy+3)
end
