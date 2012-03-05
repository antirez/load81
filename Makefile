all: load81 

load81: load81.c lua/src/liblua.a bitfont.h
	$(CC) -Ilua/src -O2 -Wall -W $< `sdl-config --cflags` `sdl-config --libs` -lm lua/src/liblua.a -o $@

lua/src/liblua.a:
	-(cd lua && $(MAKE) ansi)

clean:
	rm -f load81 

distclean: clean
	-(cd lua && $(MAKE) clean)
