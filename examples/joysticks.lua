--
-- simple demo of joystick input
--

function setup()
    background(0,0,0,0)
    cell_size = HEIGHT / 4;
end

function draw_joystick_info(joynum)
    text(10, joynum * cell_size, string.format("# %d %s x:%d/y:%d",
        joynum, joystick[joynum].name,
        joystick[joynum].x, joystick[joynum].y));
end

function draw()
    fill (0,255,0,1);
    background(0,0,0,0);

    if JOYSTICKS == 0 then
    	text(10, cell_size, string.format("No joysticks detected .. plug one in!"));
	end

    for jn = 1, JOYSTICKS, 1 do
        draw_joystick_info(jn);
    end 
end
