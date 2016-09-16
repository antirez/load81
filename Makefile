PKGS=sdl SDL_gfx SDL_image
CFLAGS=-O2 -Wall -W -Ilua/src `sdl-config --cflags` 
LDLIBS=lua/src/liblua.a -lm `sdl-config --libs` -lSDL_gfx -lSDL_image

# Customizations per-OS
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
CFLAGS+=-I/usr/local/Cellar//sdl_gfx/2.0.25/include/SDL/ -I/usr/local/Cellar//sdl_image/1.2.12_3/include/SDL/
endif

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
