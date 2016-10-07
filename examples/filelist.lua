
listed = false


function setup()
	fileList = {}
	print("current working dir: " .. lfs.currentdir())
	for file in lfs.dir[[./examples/]] do
	    if lfs.attributes(file,"mode") == "file" then 
	    	print("found file, "..file)
	    elseif lfs.attributes(file,"mode")== "directory" then
	    	if (file == ".") then
			    	print("found dir, "..file," containing:")
	    	    for l in lfs.dir("./examples/"..file) do
	    	    	if (l~="." and l~="..") then
	        	    	print("",l)
						-- table.insert(fileList, file)
	        	 	end
	        	end
	        end
	    end
	end
end


function draw()
	if (listed == false) then
		for iFile,aFile in ipairs(fileList) do
			print("a File:"..iFile)
		end
		listed = true
		else print("Listed?")
	end
end


