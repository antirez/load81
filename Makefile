PKGS=sdl SDL_gfx
CFLAGS=-O2 -Wall -W -Ilua/src `pkg-config --cflags $(PKGS)`
LDLIBS=lua/src/liblua.a -lm `pkg-config --libs $(PKGS)`

all: load81 

load81: load81.o editor.o framebuffer.o lua/src/liblua.a

lua/src/liblua.a:
	-(cd lua && $(MAKE) ansi)

clean:
	rm -f load81 *.o

distclean: clean
	-(cd lua && $(MAKE) clean)
