function setup()
    background(0,0,0);
end

function draw()
    fill(math.random(255),math.random(255),math.random(255),math.random())
    triangle(math.random(WIDTH),math.random(HEIGHT),
             math.random(WIDTH),math.random(HEIGHT),
             math.random(WIDTH),math.random(HEIGHT))
end
