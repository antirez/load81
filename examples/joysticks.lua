function setup()
    background(0,0,0,0)
end

function draw()

 background(0,0,0);
 
 fill(0,255,0,1)
 text(10,HEIGHT/2,string.format("#JOYSTICKS=%d %s x:%d/y:%d %s x:%d/y:%d %s x:%d/y:%d %s x:%d/y:%d",
    JOYSTICKS, 
    joystick[1].name, joystick[1].x, joystick[1].y,
    joystick[2].name, joystick[2].x, joystick[2].y,
    joystick[3].name, joystick[3].x, joystick[3].y,
    joystick[4].name, joystick[4].x, joystick[4].y
    ))

end
