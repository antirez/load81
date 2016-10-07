# runs load81, resetting it if any file-changes occur in the 
# WATCHDIR, for example, the onboard examples/ .. any changes
# to the dir will re-load load81 ..
#!/bin/bash
export DISPLAY=:0
WATCHDIR=./examples/
inotifywait -m -e close_write $WATCHDIR | \
while read -r notifile event filename ; \
do \
	echo "notifile: $notifile event: $event filename: $filename"
	killall load81 ;  \
	./load81 --width 480 --height 272 $notifile/$filename & \
done
