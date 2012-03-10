PKGS=sdl SDL_gfx
CFLAGS=-O2 -Wall -W -Ilua/src `pkg-config --cflags $(PKGS)`
LDLIBS=-lm lua/src/liblua.a `pkg-config --libs $(PKGS)`

all: load81 

load81.o: bitfont.h

lua/src/liblua.a:
	-(cd lua && $(MAKE) ansi)

clean:
	rm -f load81 load81.o

distclean: clean
	-(cd lua && $(MAKE) clean)
