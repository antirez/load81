
pnd-file:
	cp -rfv ../../examples/* PND_Resources/examples
	cp ../../load81 PND_Resources/
	./pnd_make.sh -c -d PND_Resources/ -x PND_Resources/PXML.xml -i PND_Resources/icon.png -p ./LOAD81.pnd

clean:
	rm -rf PND_Resources/examples/*
	rm -f PND_Resources/load81
	rm -f LOAD81.pnd
