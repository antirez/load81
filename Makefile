all: codakido

codakido: codakido.c lua/src/liblua.a
	$(CC) -Ilua/src -O2 -Wall -W $< `sdl-config --cflags` `sdl-config --libs` -lm lua/src/liblua.a -o $@

lua/src/liblua.a:
	-(cd lua && $(MAKE) ansi)

clean:
	rm -f codakido

distclean: clean
	-(cd lua && $(MAKE) clean)
