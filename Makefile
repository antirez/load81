CFLAGS = -Ilua/src -O2 -Wall

all: load81 

load81: load81.c drawing editor load81.h lua/src/liblua.a bitfont.dat
	$(CC) $(CFLAGS) `sdl-config --cflags` -c load81.c
	$(CC) *.o $(CFLAGS) `sdl-config --cflags` `sdl-config --libs` -lm lua/src/liblua.a -o load81

editor: editor.h editor.c
	$(CC) $(CFLAGS) `sdl-config --cflags` -c editor.c

drawing: drawing.h drawing.c
	$(CC) $(CFLAGS) `sdl-config --cflags` -c drawing.c

lua/src/liblua.a:
	-(cd lua && $(MAKE) ansi)

clean:
	rm -f *.o load81 

distclean: clean
	-(cd lua && $(MAKE) clean)
