-- Basic Scorched Earth clone

function setup()
	setup_terrain()
end

function setup_terrain()
	terrain = {}
    local periods = {}
    local amplitudes = {}
    local nwaves = 10
    for i = 1, nwaves do
        periods[i] = math.random()*0.1 + 0.01
        amplitudes[i] = math.random() * 50
    end
	for i = 0, WIDTH-1 do
        local h = 200
        for j = 1, nwaves do
            h = h + math.sin(i*periods[j])*amplitudes[j]
        end
        terrain[i] = h
	end
end

function draw()
	draw_terrain()
end

function draw_terrain()
	background(30, 30, 200)
    fill(20, 150, 20, 1.0)
	for i = 0, WIDTH-1 do
		line(i, 0, i, terrain[i])
	end
end
