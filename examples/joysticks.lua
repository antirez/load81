--
-- simple demo of joystick input
-- by torpor (seclorum@me.com)
--
-- the joystick[] table is global, and contains .x/.y/.name fields
-- describing the joystick.  the JOYSTICK global contains the
-- number of physical joysticks detected by LOAD81 at startup.
--
-- the maximum number of joysticks available is 4.
--

-- each joystick will have its own color, and since there 
-- a maximum of 4 joysticks available then we have 4
-- unique colors to use
colors = {}
colors = {
{r=255,g=0,b=255,a=1},
{r=0,g=0,b=255,a=1}, 
{r=255,g=0,b=0,a=1}, 
{r=0,g=255,b=0,a=1} 
}

function setup()
    print("Number of joysticks", joystick.count);
    background(0,0,0,0)
    -- divide the screen in 4, one quad for each joystick device
    joy_quad = HEIGHT / 4;
end

-- simple map function
function map(x, in_min, in_max, out_min, out_max)
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
end

function draw_joystick_info(joynum)
    -- each joystick gets its own color
    fill(colors[joynum].r,colors[joynum].g,colors[joynum].b,colors[joynum].a);
    
    -- text explaining joystick name and x/y values goes in a 'quarter' of the screen
    text(10, joynum * joy_quad, string.format("# %d %s x:%d/y:%d",
        joynum, joystick[joynum].name,
        joystick[joynum].x, joystick[joynum].y));

    -- each joystick returns x/y axes values from -32767 to 32767, so we map to 
    -- the screen size
    dot_x, dot_y = 0;
    dot_x = map(joystick[joynum].x, 32767, -32767, WIDTH, 0);
    dot_y = map(joystick[joynum].y, 32767, -32767, 0, HEIGHT);

    -- draw the joystick dot
    ellipse(dot_x, dot_y, 10, 10);
end


function draw()
    -- clear the screen
    background(0,0,0,0);

    if joystick.count == 0 then
        fill (255,0,0,1);
        text(10, joy_quad, string.format("No joysticks detected .. plug one in and try again!"));
    end

    -- draw the info for each joystick
    for jn = 1, joystick.count, 1 do
        draw_joystick_info(jn);
    end 
end