PKGS=sdl2 SDL2_gfx SDL2_image
CFLAGS=-O2 -Wall -W -Ilua/src `pkg-config --cflags $(PKGS)`
LDLIBS=lua/src/liblua.a -lm `pkg-config --libs $(PKGS)`

all: load81 

load81: load81.o editor.o framebuffer.o lua/src/liblua.a
editor.o: editor.c editor.h framebuffer.h
framebuffer.o: framebuffer.c framebuffer.h bitfont.h
load81.o: load81.c framebuffer.h editor.h load81.h

lua/src/liblua.a:
	-(cd lua && $(MAKE) ansi)

clean:
	rm -f load81 *.o

distclean: clean
	-(cd lua && $(MAKE) clean)

dep:
	$(CC) -MM *.c
