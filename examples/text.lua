function draw()
    background(0,0,0)
    fill(100,50,250,1)
    text(20,HEIGHT-40,"Please, enter this window with your mouse pointer.")
    fill(255,0,0,1)
    text(mouse.x,mouse.y,"Hello World!")
    fill(200,200,200,1)
    text(0,0,string.format("Mouse is at x:%s y:%s",mouse.x,mouse.y))
end
