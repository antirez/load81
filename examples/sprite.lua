
local x, y = 0, 0

function draw()
	background(0, 0, 0, 255)

	sprite("sprite.png", x, y);

	x = x + 3
	if (x > WIDTH) then x = -46 end

	y = y + 3
	if (y > HEIGHT) then y = -46 end
end

