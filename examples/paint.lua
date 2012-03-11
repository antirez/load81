function setup()
    background(0,0,0,0)
end

function draw()
    if mouse.pressed['1'] then
        fill(255,0,0,50)
    else
        fill(0,0,255,50)
    end
    ellipse(mouse.x,mouse.y,30,30)
end
