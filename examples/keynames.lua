function setup()
    print("Press any key to see the corrisponding name.")
end

function draw()
    for k,v in pairs(keyboard.pressed) do
        print(k)
    end
end
