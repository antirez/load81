#ifndef LOAD81_H
#define LOAD81_H

#include "framebuffer.h"

/* ============================= Data structures ============================ */

struct globalConfig {
    /* Runtime */
    int width;
    int height;
    int r,g,b;
    int alpha;
    int fps;
    long long start_ms;
    long long epoch;
    frameBuffer *fb;
    char *filename;
    lua_State *L;
    int luaerr; /* True if there was an error in the latest iteration. */
    /* Command line switches */
    int opt_show_fps;
    int opt_full_screen;
};

extern struct globalConfig l81;

#endif /* LOAD81_H */
