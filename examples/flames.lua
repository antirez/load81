-- Flames.lua, contributed by pmprog in the OpenPandora board.
-- See http://boards.openpandora.org/index.php?/topic/7405-here-is-a-pnd-for-load81/

function setup()
    local x, y, l

    MaxFlames = 150
    FlameLife = 40
    FlameSize = 10
    refreshCount = 0
    skipCount = 0
    Flames = { }

    for i=1,MaxFlames do
        x = math.random(WIDTH/3) + (WIDTH/3)
        y = math.random(FlameSize)
        l = math.random(FlameLife)
        a = { x = x, y = y, l = l }
        table.insert(Flames,a)
    end
end

function draw()
    local i, f, minMove

    background(0,0,0)
    for i,f in pairs(Flames) do
        if f.l > 35 then
            fill(255, 255, 255, 230)
            minMove = 0
        elseif f.l > 30 then
            fill(255, 255, 192, 205)
            minMove = 1
        elseif f.l > 20 then
            fill(255, 192, 128, 178)
            minMove = 2
        elseif f.l > 10 then
            fill(220, 128, 100, 128)
            minMove = 3
        else
            fill(160, 128, 80, 76)
            minMove = 5
        end

        ellipse(f.x,f.y,FlameSize,FlameSize)
        f.l = f.l - math.random(3)
        if f.l <= 0 then
            f.x = math.random(WIDTH/3) + (WIDTH/3)
            f.y = math.random(FlameSize)
            f.l = FlameLife
        else
            f.y = f.y + (math.random(6) + minMove)
            f.x = f.x + math.random(7) - 3
        end
    end
end

