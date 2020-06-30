-- astropixel - getpixel/setpixel demo / k255 | 3x /

function setup()
    px,py=WIDTH/2,HEIGHT/2
    dpx,dpy=4,4
    sx,sy={},{}
    sn=1024
    for i=1,sn do
        sx[i],sy[i]=math.random(1,WIDTH),math.random(1,HEIGHT)
    end
    pc,sc={ 255,255,255,1 },{ 255,0,255,1 }
    tick=0
    setFPS(60)
end

function draw()
    tick=tick+1
    fill(0,0,0,0.05)
    rect(0,0,WIDTH,HEIGHT)
    fill(unpack(sc))
    for i=1,sn do
        if tick%60==0 and math.random(0,10)==10 then
          ellipse(sx[i],sy[i],2,2)
        end
        ellipse(sx[i],sy[i],1,1)
    end
    fill(unpack(pc))
    if getpixel(px,py) == unpack(sc) then
        dpx,dpy=math.random(-10,10)/5,math.random(-10,10)/5
    end
    setpixel(px,py)
    px,py=(px+dpx)%WIDTH,(py+dpy)%HEIGHT
end
