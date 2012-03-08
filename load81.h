#ifndef __LOAD81__
#define __LOAD81__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <SDL.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <errno.h>
#include <ctype.h>
#include "drawing.h"
#include "editor.h"

#define NOTUSED(V) ((void) V)

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

struct globalConfig {
    /* Runtime */
    int width;
    int height;
    int r,g,b;
    float alpha;
    long long epoch;
    SDL_Surface *screen;
    frameBuffer *fb;
    lua_State *L;
    unsigned char *font[256];
    char *filename;
    char *luaerr;
    int luaerrline;
} l81;

#endif
