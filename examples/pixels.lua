-- pixels - setpixel demo / k255 | 3x /

function setup()
    tick=0
    d=1000
    s=50
    rgba={ 255,255,255,.75 }
end

function draw()
    tick=tick+1
    fill(unpack(rgba))
    if tick%s==0 then
        rgba[math.random(1,3)] = math.random(0,255)
    end
    for i=0,d,1 do
      setpixel(math.random(0,WIDTH),math.random(0,HEIGHT))
    end
end
