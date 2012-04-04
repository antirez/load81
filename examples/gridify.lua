 --
-- draw some grids
--
screen_width = WIDTH
screen_height = HEIGHT
screen_w_center = screen_width / 2
screen_h_center = screen_height / 2
 
colors = {}
colors = { {r=255,  g=0,    b=255,  a=1},
           {r=0,    g=0,    b=255,  a=1}, 
           {r=255,  g=0,    b=0,    a=1}, 
           {r=0,    g=255,  b=0,    a=1},
           {r=27,   g=73,   b=100,  a=1},
           {r=0,    g=51,   b=75,   a=1},
           {r=100,  g=35,   b=15,   a=1},
           {r=200,  g=135,  b=150,  a=1},
           {r=100,  g=0,    b=0,    a=1},
           {r=100,  g=50,   b=0,    a=1},
           {r=153,  g=150,  b=50,   a=1},
           {r=25,   g=102,  b=0,    a=1},
           {r=25,   g=102,  b=0,    a=1},
           {r=80,   g=500,  b=200,  a=1} }


-- dunno where we should get this from .. math.??
function map(x, in_min, in_max, out_min, out_max)
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
end
 
function setup()
    background(0,0,0,0)
    cell_size = HEIGHT / 4
    fill (27,73,100,1)
    background(0,0,0,0)
    text(10, 10, string.format("gridify, by torpor"))
end

function draw_XY_grid()
    for i = 0, screen_width, 10 do
        for screen_w_center = 0, screen_height do 
            if (((i % 50) == 0) and ((screen_w_center % 50) == 0)) then

                fill(0, 51, 75, 1)
                text(i+4, screen_w_center-1, tostring(i))
       
                fill(100, 35, 15, 1)
                text( i+4, screen_w_center+8, tostring(i)) -- padding for y direction
                rect(i, screen_w_center, 4, 4)

            end
      
            if ((screen_w_center % 10) == 0)  then

                fill(200, 135, 150, 1)
                rect(i,screen_w_center,1,1)

            end
        end
    end
end

function draw_centered_grid()
    
    fill(100,0,0,1)
    rect(screen_w_center, screen_h_center, 4, 4)
  
    fill (100,50,0,1)
    --fill (153,150,50)
    line(0, screen_h_center, screen_width, screen_h_center)
    line(screen_w_center, 0, screen_w_center, screen_height)
   
    for z = screen_w_center,  screen_width, 10 do
        if (((z - screen_w_center) % 50) == 0) then
            fill (25,102,0,1)
            ellipse(z, screen_h_center, 6, 6)
            --text(z - 8, screen_h_center + 16 ,tostring(z - screen_w_center))   -- padding
        end
    end
    
    for z = screen_w_center, 0, -10 do
        if (((z - screen_w_center) % 50) == 0) then 
            fill (25,102,0,1)
            ellipse(z, screen_h_center, 6, 6)
            --text(z - 8, screen_h_center + 16,  tostring(z - screen_w_center))  -- padding
        end
    end     

    for z = screen_h_center, screen_height, 10 do
        if (((z - screen_h_center) % 50) == 0) then 

            ellipse(screen_w_center, z, 6, 6)
            --text(screen_w_center - 8, z + 16, tostring(z - screen_h_center))  -- padding
        end
    end

    for z = screen_h_center, 0, -10 do
        if (((z - screen_h_center) % 50) == 0) then
            --text(screen_w_center - 8, z + 16, tostring(z - screen_h_center)) -- padding
        end
     end
end

--[[
// This draws the 'percentage' scale for the centered grid, using
// the largest axes as the base to determine the appropriate
// interval
]]
function draw_percentile_grid()

    biggest_axes = math.max (screen_width, screen_height)
    smallest_axes = math.min (screen_width, screen_height)
    difference_between_axes = ((biggest_axes - smallest_axes) / 2)
    difference = 0

    if screen_width == biggest_axes then
        difference = 0
    else
        difference = difference_between_axes
    end
  
    fill(80,500,200,1)

    i=0  
    for i=0, 100, 5 do
        -- map along the x axis first
        x_loc = map(i, 0, 100, 0, biggest_axes) - difference
        rect(x_loc, screen_h_center, 5, 5)
        text(x_loc -25, screen_h_center+6, tostring(i - 50))  
    end
    
    if (screen_height == biggest_axes)  then
        difference = 0
    else
        difference = difference_between_axes
    end
    
    for i=0, 100, 5 do -- map along the y axis next
        y_loc = map(i, 0, 100, 0, biggest_axes) - difference
        rect(screen_w_center, y_loc, 5, 5)
        text(screen_w_center-25, y_loc + 6, tostring((i - 50) * -1) .. "%") 
    end
end
 

function draw_colors_grids()
print("screen_w_center : ", screen_w_center)
print("screen_width : ", screen_width)
    for i = 1, #colors do
        for x = screen_w_center, screen_width do
            for y = screen_h_center, screen_height do
                fill (colors[i].r, colors[i].g, colors[i].b, colors[i].a)
                rect (x * 10, y * 10, 5, 5)
            end
        end
    end
end


function draw_grids()
    draw_percentile_grid()
    draw_centered_grid()
    draw_XY_grid()
--    draw_colors_grids()
end

function draw()
   draw_grids()
end

