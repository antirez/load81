 --
-- display time in all its joy
--
screen_width = WIDTH
screen_height = HEIGHT
screen_w_center = screen_width / 2
screen_h_center = screen_height / 2
cell_size = HEIGHT / 4
 
colors = {}
colors = { {r=255,  g=0,    b=255,  a=1},
           {r=0,    g=0,    b=255,  a=1}, 
           {r=255,  g=0,    b=0,    a=1}, 
           {r=80,   g=500,  b=200,  a=1},
           {r=87, g=133, b=160, a =1} }

NUM_EVENTS = 20

SLICE_MODULO = 1

-- present-time countdown
ptc = NUM_EVENTS

-- current frame values only
tick = 0
time_str = "time: "
pixlen = 0
 
-- from now until NUM_EVENTS
event_ticks = {}
event_joyheights = {}

-- dunno where we should get this from .. math.??
function map(x, in_min, in_max, out_min, out_max)
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
end
 
function setup()
    background(255, 0, 0, 0)
    tick = tick + 1

    fill (27,73,100,1)
    background(0,0,0,0)

    text(10, 10, string.format("timeout, by torpor"))
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
            text(z - 8, screen_h_center + 16 ,tostring(z - screen_w_center))   -- padding
        end
    end
    
    for z = screen_w_center, 0, -10 do
        if (((z - screen_w_center) % 50) == 0) then 
            fill (25,102,0,1)
            ellipse(z, screen_h_center, 6, 6)
            text(z - 8, screen_h_center + 16,  tostring(z - screen_w_center))  -- padding
        end
    end     

    for z = screen_h_center, screen_height, 10 do
        if (((z - screen_h_center) % 50) == 0) then 

            ellipse(screen_w_center, z, 6, 6)
            text(screen_w_center - 8, z + 16, tostring(z - screen_h_center))  -- padding
        end
    end

    for z = screen_h_center, 0, -10 do
        if (((z - screen_h_center) % 50) == 0) then
            ellipse(screen_w_center, z, 6, 6)
            text(screen_w_center - 8, z + 16, tostring(z - screen_h_center)) -- padding
        end
     end
end


function draw_colors_grids()
    for x = 1, screen_width / cell_size do
        for y = 1, screen_height / cell_size do
            for i = #colors, 1, -1 do
                fill (colors[i].r, colors[i].g, colors[i].b, colors[i].a)
                --print(" colors: " .. colors[i].r .. " " ..  colors[i].g .. " " ..  colors[i].b .. " " .. colors[i].a)
                --rect (x * (cell_size / 16), y * (cell_size / 16), 15, 15)
                rect (x * 16, y * 16, i * 15, i * 15)
            end
        end
    end
end

function draw_event_labels(i) 
	fill(colors[2].r, colors[2].g, colors[2].b, colors[2].a)

	rect( (i * 48) - 38, 
		  screen_h_center,
		  (i * 48) - 34, 
		  map(event_joyheights[i], -32768, 32768, cell_size * 2, -cell_size * 2))

    fill (87,133,160,1)
    text((i * 48) - 38, screen_h_center, event_ticks[i] .. " ") 
end
 
function draw()
    -- we keep our own count of event_ticks since start
    tick = tick + 1

    -- keep a list of NUM_EVENTS records and therefore need a present counter
    if ptc <= 1 then 
        ptc = NUM_EVENTS
    end

    -- decrement 
    ptc = ptc - 1

    -- time slice
    if ((ptc % SLICE_MODULO) == 0) then

        background(0,0,0,1)

        --draw_percentile_grid()
        draw_centered_grid()
        --draw_XY_grid()

        -- remember our current ptc data
        table.insert(event_ticks, tick)
		table.insert(event_joyheights, joystick[1].y)

        for i = #event_ticks,1,-1 do
            draw_event_labels(i)
        end

        draw_colors_grids()

        -- prune our little stack
        if #event_ticks > NUM_EVENTS then
          table.remove(event_ticks, 1)
		  table.remove(event_joyheights, 1)
        end

    end

end

