-- [[
-- Small helicopter game.
-- by Jan-Erik Rediger (badboy_)
--
-- Have fun!
-- ]]

function setup()
    ticks = 0 -- Number of iteration of the program
    heli = {  -- Heli object
        x = 100,
        y = HEIGHT/2 - 30,
        w = 100,
        h = 48,
    }
    speed = 0 -- Current speed, positive: up, negative: down
    max_speed = 4 -- Maximum speed in both directions
    hit = 0 -- Did we hit a border/block?
    points = 0 -- Current points

    started = 0 -- Press Enter to start
    last_pressed = 0

    -- create initial game field
    top_border = createBorder()
    bottom_border = createBorder()
    block = createBlock()

    clear()
end

function newBorder(x,h,w)
    return {x = x, h = h, w = w}
end

function createBorder()
    bord = {}
    local x = 0
    while x < WIDTH do
        local h = math.random(30, 100)
        local w = math.random(5, 40)

        table.insert(bord, newBorder(x,h,w))

        x = x+w
    end

    return bord
end

function createBlock()
    local w = math.random(70, 200)
    local h = math.random(70, 200)

    return {x = WIDTH, w = w, h = h}
end

function clear()
    background(0,0,0)
    fill(255, 255, 255, 1)
end

function drawBorder(arr, top)
    fill(127, 127, 127, 1)
    for i,b in pairs(arr) do

        if top == 1 then
            rect(b.x, HEIGHT-b.h, b.w, b.h)
        else
            rect(b.x, 0, b.w, b.h)
        end
    end
end

function moveBlock()
    if hit == 1 then return end
    if block == nil then
        block = createBlock()
        return
    end

    block.x = block.x-2
    if block.x+block.w < 0 then
        block = nil
    end
end

function moveBorder(border_arr)
    table.remove(border_arr, 1)
    local new_arr = {}
    local rem = 0
    for i,b in pairs(top_border) do
        if i == 1 then
            rem = b.x
        end

        b.x = b.x-rem
        last_x = b.x+b.w

        table.insert(new_arr, b)
    end
    table.insert(new_arr, newBorder(last_x, math.random(30, 100), WIDTH-last_x))
    return new_arr
end

function moveBorders()
    if hit == 1 or ticks%4 ~= 0 then return end
    top_border = moveBorder(top_border)
    bottom_border = moveBorder(bottom_border)
    points = points + 1
end

-- Math formula to rotate a point counterclockwise, with
-- rotation center at 0,0.
function rotatePoint(x,y,a)
    return x*math.cos(a)-y*math.sin(a),
           y*math.cos(a)+x*math.sin(a)
end

-- Math formula to rotate a point counterclockwise, with
-- given rotation center.
function rotateCenterPoint(x, y, center_x, center_y, a)
    local origin = { x = x - center_x, y = y - center_y }
    local nx, ny = rotatePoint(origin.x, origin.y, a)

    return nx+center_x, ny+center_y
end

function drawHeli()
    x = heli.x
    y = heli.y
    fill(255, 255, 255, 1)

    --[[
    --        ---------
    --      /  ---|---
    --     /---|       \
    --    /    |________\
    --          _|___|_
    --]]
    

    -- (debug) collision box
    --fill(0, 0, 255, 0.5)
    --rect(x, y, heli.w, heli.h)

    fill(255, 255, 255, 1)
    -- body
    rect(x+30, y+10, 50, 30)
    triangle(x+30+51, y+10, x+30+52, y+10+30, x+30+50+20, y+9)

    -- back
    rect(x+5, y+10+15, 25, 2)
    --if ticks%5 == 0 then
        --fill(255, 255, 255, 255)
    --else
        --fill(153, 153, 153, 255)
    --end

    local center_x = x+5
    local center_y = y+10+16.5

    local rotate_x1
    local rotate_y1
    local rotate_x2
    local rotate_y2

    if hit == 1 or started == 0 then
        rotate_x1 = x
        rotate_y1 = y+10+5

        rotate_x2 = x+10
        rotate_y2 = y+10+28
    else
        rotate_x1, rotate_y1 = rotateCenterPoint(x, y+10+5, center_x, center_y, -(ticks/3))
        rotate_x2, rotate_y2 = rotateCenterPoint(x+10, y+10+28, center_x, center_y, -(ticks/3))
    end

    line(rotate_x1, rotate_y1, rotate_x2, rotate_y2)

    -- bottom
    fill(255, 255, 255, 255)
    line(x+30+15, y, x+30+15, y+10)
    line(x+30+45, y, x+30+45, y+10)
    line(x+35, y, x+35+50, y)

    -- top
    if ticks%5 == 0 then
        fill(255, 255, 255, 255)
    else
        fill(153, 153, 153, 255)
    end
    line(x+30+30, y+10+30, x+30+30, y+10+30+7)
    line(x+25, y+10+37, x+25+35+35, y+10+37)
end

function speedUp()
    if speed < max_speed then
        speed = speed + 1
    end
end

function speedDown()
    if speed > -max_speed then
        speed = speed - 1
    end
end

function in_corridor(x, xw, cx, cw)
    if (cx >= x and cx <= xw) or
        (cw >= x and cw <= xw) or
        (cx < x and cw > xw) then
        return true
    end
end

function detecCollision()
    -- upper half
    if heli.y > HEIGHT/2 then 
        for i,b in pairs(top_border) do
            if in_corridor(b.x, b.x+b.w, heli.x, heli.x+heli.w) and
                heli.y+heli.h >= HEIGHT-b.h then
                hit = 1
                return
            end
        end
    else -- lower half
        for i,b in pairs(bottom_border) do
            if in_corridor(b.x, b.x+b.w, heli.x, heli.x+heli.w) and
                heli.y <= b.h then
                hit = 1
                return
            end
        end
    end

    if block == nil then return end

    local by = (HEIGHT/2)-(block.h/2)

    -- hit from the left
    if heli.x+heli.w >= block.x and heli.x <= block.x+block.w and heli.y >= by and heli.y <= by+block.h then
        hit = 1
        return
    end

    -- hit from the top
    if heli.y > by and heli.x+heli.w >= block.x and heli.x <= block.x+block.w and 
        heli.y <= by+block.h then
        hit = 1
        return
    end

    -- hit from the bottom
    if heli.y < by and heli.x+heli.w >= block.x and heli.x <= block.x+block.w and
        heli.y+heli.h >= by then
        hit = 1
        return
    end
end

function drawBorders()
    drawBorder(top_border, 1) 
    drawBorder(bottom_border, 0) 
end

function drawPoints()
    fill(255,0,0,1)
    text(WIDTH-50,HEIGHT-20, string.format("%d", points))
end

function drawBlock()
    if block == nil then return end

    fill(128,127,127,1)
    rect(block.x, (HEIGHT/2)-(block.h/2), block.w, block.h)
end

function draw()
    ticks = ticks+1
    clear()

    if keyboard.pressed['return'] then 
        if ticks-last_pressed < 5 then return end
        last_pressed = ticks
        if started == 0 then started = 1
        else started = 0 end
    end

    if hit == 0 and started == 0 then
        drawBorders() 
        drawBlock()
        drawPoints()

        text((WIDTH/2)-60,HEIGHT/2, "paused. press enter.")
        drawHeli()
        return
    end

    -- Handle keyboard events.
    if keyboard.pressed['space'] then 
        speedUp()
    else
        speedDown()
    end

    if hit == 1 then
        speed = 0
        fill(255,0,0,1)
        text((WIDTH/2)-40,HEIGHT/2, "we got hit")
    end

    heli.y = heli.y + speed
    moveBorders()
    moveBlock()

    drawBorders()
    drawPoints()
    drawBlock()
    drawHeli()

    detecCollision()
end
