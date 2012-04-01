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
    filled(false)

    for i=1,MaxFlames do
        x = math.random(WIDTH/3) + (WIDTH/3)
        y = math.random(FlameSize)
        l = math.random(FlameLife)
        a = { x = x, y = y, l = l }
        table.insert(Flames,a)
    end
end

-- can't find this in .math, but lets have it here for later transformation anyway ..
function map(x, in_min, in_max, out_min, out_max)
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
end
 
function draw()
    local i, f, minMove;

	-- user can move joystick to get different flame effects
	local joy_Lengths = { 5, 15, 20, 45, 60, 95, 140, 170, 200};
	local joy_Flames = { 5, 10, 15, 30, 60, 75, 90, 150 };

    background(0,0,0)

	-- ahem .. change properties of the flame according to joystick input
	FlameSize = joy_Lengths[math.floor(map (joystick[1].x, 32767, -32767, 1, #joy_Lengths) + .5)]
	FlameLife = joy_Flames[math.floor(map (joystick[1].y, 32767, -32767, 1, #joy_Flames) + .5)]

    ellipse(dot_x, dot_y, 30, 20);
 
    for i,f in pairs(Flames) do
        if f.l > 35 then
            filled(true)
            fill(255, 255, 255, 0.9)
            minMove = 0
        elseif f.l > 30 then
            filled(false)
            fill(255, 255, 192, 0.8)
            minMove = 1
        elseif f.l > 20 then
            fill(255, 192, 128, 0.7)
            minMove = 2
        elseif f.l > 10 then
            fill(220, 128, 100, 0.5)
            minMove = 3
        else
            fill(160, 128, 80, 0.3)
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

