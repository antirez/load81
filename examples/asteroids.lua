-- Simple Asteroids-alike game.
-- Copyright (C) 2012 Salvatore Sanfilippo.
-- This code is released under the BSD two-clause license.

function setup()
    ticks = 0 -- Number of iteration of the program
    shipx = WIDTH/2 -- Ship x position
    shipy = HEIGHT/2 -- Ship y position
    shipa = 0 -- Ship rotation angle
    shipvx = 0 -- Ship x velocity
    shipvy = 0 -- Ship y velocity
    bullets = {} -- An array of bullets
    asteroids = {} -- An array of asteroids
    asteroids_num = 0; -- Number of asteroids on screen
    asteroids_max = 2; -- Max number of asteroids to show
    last_bullet_ticks = 0 -- Ticks at the time the last bullet was fired

    -- Populate the game with asteroids at start
    while(asteroids_num < asteroids_max) do
        addAsteroid()
    end
end

-- The following functions move objects adding the velocity
-- to the position at every iteration.

function moveShip()
    shipx = (shipx + shipvx) % WIDTH
    shipy = (shipy + shipvy) % HEIGHT
end

function moveBullets()
    local i,b
    for i,b in pairs(bullets) do
        b.x = b.x+b.vx
        b.y = b.y+b.vy
        b.ttl = b.ttl - 1
        if b.ttl == 0 then bullets[i] = nil end
    end
end

function moveAsteroids()
    local i,a
    for i,a in pairs(asteroids) do
        a.x = (a.x+a.vx) % WIDTH
        a.y = (a.y+a.vy) % HEIGHT
    end
end

--  Add an asteroid. Create a random asteroid so that it's
--  not too close to the ship.

function addAsteroid()
    local x,y,a,ray
    while true do
        x = math.random(WIDTH)
        y = math.random(HEIGHT)
        ray = math.random(20,40)
        if math.abs(x-shipx) > ray and
           math.abs(y-shipy) > ray then
            break
        end
    end
    a = { x = x, y = y, vx = math.random()*2, vy = math.random()*2,
          ray = ray }
    table.insert(asteroids,a)
    asteroids_num = asteroids_num + 1
end

-- Fire a bullet with velocity 2,2 and the same orientation
-- as the ship orientation.

function fire()
    local b
    -- Don't fire again if we already fired
    -- less than 5 iterations ago.
    if ticks - last_bullet_ticks < 5 then return end
    b = { x = shipx, y = shipy,
          vx = shipvx+(4*math.sin(shipa)),
          vy = shipvy+(4*math.cos(shipa)),
          ttl=300 }
    -- Make sure that the bullet originaes from ship head
    b.x = b.x+(20*math.sin(shipa))
    b.y = b.y+(20*math.cos(shipa))
    -- Finally insert the bullet in the table of bullets
    table.insert(bullets,b)
    last_bullet_ticks = ticks
end

-- Draw the screen, move objects, detect collisions.
function draw()
    ticks = ticks+1

    -- Handle keyboard events.
    if keyboard.pressed['left'] then shipa = shipa - 0.1 end
    if keyboard.pressed['right'] then shipa = shipa + 0.1 end
    if keyboard.pressed['up'] then
        shipvx = shipvx + 0.15*math.sin(shipa)
        shipvy = shipvy + 0.15*math.cos(shipa)
    end
    if keyboard.pressed['space'] then fire() end

    -- Create a new asteroid from time to time if needed
    if asteroids_num < asteroids_max and (ticks % 200) == 0 then
        while(asteroids_num < asteroids_max) do addAsteroid() end
    end

    -- Move all the objects of the game.
    moveShip()
    moveBullets()
    moveAsteroids()
    checkBulletCollision()

    -- Draw the current game screen.
    background(0,0,0)
    drawBullets()
    drawAsteroids()
    drawShip(shipx,shipy,shipa)
end

-- Math formula to rotate a point counterclockwise, with
-- rotation center at 0,0.

function rotatePoint(x,y,a)
    return x*math.cos(a)-y*math.sin(a),
           y*math.cos(a)+x*math.sin(a)
end

-- Draw the ship, that is composed of three vertex,
-- rotating the vertexes using rotatePoint().

function drawShip(x,y,a)
    local triangles = {}
    table.insert(triangles,
        {x0 = -10, y0 = -10, x1 = 0, y1 = 20, x2 = 10, y2 = -10,
          r = 255, g = 0, b = 0, alpha = 1 })
    if keyboard.pressed['up'] then
        table.insert(triangles,
        {x0 = -5, y0 = -10, x1 = 0, y1 = math.random(-25,-20), x2 = 5, y2 = -10,
          r = 255, g = 255, b = 0, alpha = 1 })
    end
    for i,t in pairs(triangles) do
        fill(t.r,t.g,t.b,t.alpha)
        t.x0,t.y0 = rotatePoint(t.x0,t.y0,-a);
        t.x1,t.y1 = rotatePoint(t.x1,t.y1,-a);
        t.x2,t.y2 = rotatePoint(t.x2,t.y2,-a);
        triangle(x+t.x0,y+t.y0,x+t.x1,y+t.y1,x+t.x2,y+t.y2)
    end
end

-- Draw a bullet, that's just a single pixel.

function drawBullets()
    local i,b
    for i,b in pairs(bullets) do
        fill(255,255,255,1)
        rect(b.x-1,b.y-1,3,3)
    end
end

-- Draw an asteroid.

function drawAsteroids()
    local i,a
    for i,a in pairs(asteroids) do
        fill(150,150,150,1)
        ellipse(a.x,a.y,a.ray,a.ray)
    end
end

-- This function detects the collision between a bullet
-- and an asteroid, and removes both the bullet and the
-- asteroid from the game when they collide.

function checkBulletCollision()
    local i,j,b,a,del_asteroids,del_bullets
    del_asteroids = {}
    del_bullets = {}
    for i,b in pairs(bullets) do
        for j,a in pairs(asteroids) do
            local distance,dx,dy
            dx = a.x-b.x
            dy = a.y-b.y
            distance = math.sqrt((dx*dx)+(dy*dy))
            if distance < a.ray then
                del_asteroids[j] = true
                del_bullets[i] = true
                break
            end
        end
    end
    for i,b in pairs(del_bullets) do
        table.remove(bullets,i)
    end
    for i,a in pairs(del_asteroids) do
        table.remove(asteroids,i)
        asteroids_num = asteroids_num - 1
        if asteroids_num == 0 then
            asteroids_max = asteroids_max + 1
        end
    end
end
