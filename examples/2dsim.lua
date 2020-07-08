-- 2dsim, very basic 2D physics simulator
-- Written by Salvatore Sanfilippo and Enea Sanfilippo

function setup()
    -- x and y are the current ball position.
    x = WIDTH/2
    y = HEIGHT/2
    -- vx, vy is the velocity.
    vx = 0
    vy = 0
    -- ax, ay is the acceleration.
    ax = 0
    ay = 0
    -- ground is the y coordinate of the ground line.
    -- radius is the radius of the ball.
    GROUND = 50
    RADIUS = 10
end

function draw()
    -- Retrace the scene, writing the ground as a blue box.
    background(0,0,0)
    fill(0,0,255,1)
    rect(0,0,WIDTH,GROUND-RADIUS)

    -- Recompute the position using the velocity, also recompute the
    -- velocity using the acceleration.
    x = (x + vx) % WIDTH
    y = y + vy
    vx = vx + ax
    vy = vy + ay

    -- Let the user accelerate the body using arrow keys.
    ax = 0
    ay = 0
    if keyboard.pressed['left'] then
        ax = -1
    end
    if keyboard.pressed['right'] then
        ax = 1
    end
    if keyboard.pressed['up'] then
        ay = 1
    end
    if keyboard.pressed['down'] then
        ay = -1
    end

    -- Simulate gravity, always adding a negative Y acceleration.
    ay = ay - 0.5

    -- Detect contact with the ground.
    if y < GROUND then
        -- When the ball hits the ground, invert the sign of the y
        -- velocity in order to simulate bouncing.
        -- However only let 50% of the speed to be retained.
        y = GROUND
        vy = -vy * 0.5
        -- Filter small y velocity to avoid the ball to bounce forever.
        if (math.abs(vy) < 1) then
            vy = 0
        end
    end

    -- Simulate friction with the ground by reducign the X speed when
    -- the ball touches the ground.
    if y == GROUND then
        vx = vx * 0.95
    end

    -- Draw the ball at the new position.
    fill(255,255,0,1)
    ellipse(x,y,RADIUS,RADIUS)
end
