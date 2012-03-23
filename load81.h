#ifndef LOAD81_H
#define LOAD81_H

#include "framebuffer.h"

/* ================================ Defaults ================================ */

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600
#define DEFAULT_BPP 24

/* ============================= Data structures ============================ */

struct globalConfig {
    /* Runtime */
    int r,g,b;
    int alpha;
    char filled;
    int fps;
    long long start_ms;
    long long epoch;
    frameBuffer *fb;
    char *filename;
    lua_State *L;
    int luaerr; /* True if there was an error in the latest iteration. */
    /* Configuration */
    int width;
    int height;
    int bpp;
    /* Command line switches */
    int opt_show_fps;
    int opt_full_screen;
};

extern struct globalConfig l81;

#endif /* LOAD81_H */
