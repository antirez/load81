local t, x = 0, 0

function draw()
    background(0, 0, 0)
    local c = math.abs(math.cos(2*3.14*t));
    local y = 0.5*HEIGHT*c/math.exp(0.8*t);

    sprite("examples/sprite.png", x, y,t*-150);
    x = x + 3
    t = t + 0.01

    if (x > WIDTH) then 
        t, x = 0, 0
    end
end
