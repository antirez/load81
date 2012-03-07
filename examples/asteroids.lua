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
    a = { x = x, y = y, vx = math.random(), vy = math.random(),
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
          vx = shipvx+(2*math.sin(shipa)),
          vy = shipvy+(2*math.cos(shipa)),
          ttl=300 }
    table.insert(bullets,b)
    last_bullet_ticks = ticks
end

-- Draw the screen, move objects, detect collisions.
function draw()
    ticks = ticks+1

    -- Handle keyboard events.
    if keyboard.pressed['left'] then shipa = shipa - 0.05 end
    if keyboard.pressed['right'] then shipa = shipa + 0.05 end
    if keyboard.pressed['up'] then
        shipvx = shipvx + 0.05*math.sin(shipa)
        shipvy = shipvy + 0.05*math.cos(shipa)
    end
    if keyboard.pressed['space'] then fire() end

    -- Create a new asteroid from time to time if needed
    if asteroids_num < asteroids_max and (ticks % 500) == 0 then
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
    local x0,y0,x1,y1,x2,y2

    fill(255,0,0,1)
    x0 = -10; y0 = -10;
    x1 = 0;   y1 = 20;
    x2 = 10;  y2 = -10;
    x0,y0 = rotatePoint(x0,y0,-a);
    x1,y1 = rotatePoint(x1,y1,-a);
    x2,y2 = rotatePoint(x2,y2,-a);
    triangle(x+x0,y+y0,x+x1,y+y1,x+x2,y+y2)
end

-- Draw a bullet, that's just a single pixel.

function drawBullets()
    local i,b
    for i,b in pairs(bullets) do
        fill(255,255,255,1)
        rect(b.x,b.y,1,1)
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
        asteroids_num = asteroids_num -1
    end
end
