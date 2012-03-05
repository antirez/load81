function setup()
    print("Press any key to see the corresponding name.")
end

function draw()
    for k,v in pairs(keyboard.pressed) do
        print(k)
    end
end
