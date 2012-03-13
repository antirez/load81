--
-- simple demo of joystick input
--

colors = {}
colors = {
{r=255,g=0,b=255,a=1},
{r=0,g=0,b=255,a=1}, 
{r=255,g=0,b=0,a=1}, 
{r=0,g=255,b=0,a=1} 
}

function setup()
    background(0,0,0,0)
    cell_size = HEIGHT / 4;
end

function map(x, in_min, in_max, out_min, out_max)
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
end

function draw_joystick_info(joynum)
    fill(colors[joynum].r,colors[joynum].g,colors[joynum].b,colors[joynum].a);
    text(10, joynum * cell_size, string.format("# %d %s x:%d/y:%d",
        joynum, joystick[joynum].name,
        joystick[joynum].x, joystick[joynum].y));

    dot_x, dot_y = 0;
    dot_x = map(joystick[joynum].x, 32767, -32767, WIDTH, 0);
    dot_y = map(joystick[joynum].y, 32767, -32767, 0, HEIGHT);
    ellipse(dot_x, dot_y, 10, 10);

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
