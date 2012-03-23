#include "framebuffer.h"

static unsigned char *BitmapFont[256];

/* ============================= Frame buffer ============================== */
SDL_Surface *sdlInit(int width, int height, int bpp, int fullscreen) {
    int flags = SDL_SWSURFACE;
    SDL_Surface *screen;

    if (fullscreen) flags |= SDL_FULLSCREEN;
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        fprintf(stderr, "SDL Init error: %s\n", SDL_GetError());
        return NULL;
    }
    atexit(SDL_Quit);
    screen = SDL_SetVideoMode(width,height,bpp,flags);
    if (!screen) {
        fprintf(stderr, "Can't set the video mode: %s\n", SDL_GetError());
        return NULL;
    }
    /* Unicode support makes dealing with text input in SDL much simpler as
     * keys are translated into characters with automatic support for modifiers
     * (for instance shift modifier to print capital letters and symbols). */
    SDL_EnableUNICODE(SDL_ENABLE);
    return screen;
}

frameBuffer *createFrameBuffer(int width, int height, int bpp, int fullscreen) {
    frameBuffer *fb = malloc(sizeof(*fb));

    fb->width = width;
    fb->height = height;
    fb->screen = sdlInit(width,height,bpp,fullscreen);
    SDL_initFramerate(&fb->fps_mgr);
    /* Load the bitmap font */
    bfLoadFont((char**)BitmapFont);
    return fb;
}

void setPixelWithAlpha(frameBuffer *fb, int x, int y, int r, int g, int b, int alpha) {
    pixelRGBA(fb->screen, x, fb->height-1-y, r, g, b, alpha);
}

void fillBackground(frameBuffer *fb, int r, int g, int b) {
    boxRGBA(fb->screen, 0, 0, fb->width-1, fb->height-1, r, g, b, 255);
}

/* ========================== Drawing primitives ============================ */

void drawHline(frameBuffer *fb, int x1, int x2, int y, int r, int g, int b, int alpha) {
    hlineRGBA(fb->screen, x1, x2, fb->height-1-y, r, g, b, alpha);
}

void drawEllipse(frameBuffer *fb, int xc, int yc, int radx, int rady, int r, int g, int b, int alpha) {
    filledEllipseRGBA(fb->screen, xc, fb->height-1-yc, radx, rady, r, g, b, alpha);
}

void drawBox(frameBuffer *fb, int x1, int y1, int x2, int y2, int r, int g, int b, int alpha) {
    boxRGBA(fb->screen, x1, fb->height-1-y1, x2, fb->height-1-y2, r, g, b, alpha);
}

void drawTriangle(frameBuffer *fb, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int alpha) {
    filledTrigonRGBA(fb->screen, x1, fb->height-1-y1, x2, fb->height-1-y2, x3, fb->height-1-y3, r, g, b, alpha);
}

void drawLine(frameBuffer *fb, int x1, int y1, int x2, int y2, int r, int g, int b, int alpha) {
    lineRGBA(fb->screen, x1, fb->height-1-y1, x2, fb->height-1-y2, r, g, b, alpha);
}

void drawPolygon(frameBuffer *fb, Sint16* xv, Sint16* yv, int n, int filled, int r, int g, int b, int alpha) {
    int i;
    for (i=0; i<n; i++) yv[i] = fb->height-1-yv[i];
    if (filled) {
      filledPolygonRGBA(fb->screen, xv, yv, n, r, g, b, alpha);
    }
    else {
      polygonRGBA(fb->screen, xv, yv, n, r, g, b, alpha);
    }
}

/* ============================= Bitmap font =============================== */
void bfLoadFont(char **c) {
    /* Set all the entries to NULL. */
    memset(c,0,sizeof(unsigned char*)*256);
    /* Now populate the entries we have in our bitmap font description. */
    #include "bitfont.h"
}

void bfWriteChar(frameBuffer *fb, int xp, int yp, int c, int r, int g, int b, int alpha) {
    int x,y;
    unsigned char *bitmap = BitmapFont[c&0xff];

    if (!bitmap) bitmap = BitmapFont['?'];
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            int byte = (y*16+x)/8;
            int bit = x%8;
            int set = bitmap[byte] & (0x80>>bit);

            if (set) setPixelWithAlpha(fb,xp+x,yp-y+15,r,g,b,alpha);
        }
    }
}

void bfWriteString(frameBuffer *fb, int xp, int yp, const char *s, int len, int r, int g, int b, int alpha) {
    int i;

    for (i = 0; i < len; i++)
        bfWriteChar(fb,xp-((16-FONT_KERNING)/2)+i*FONT_KERNING,yp,
                    s[i],r,g,b,alpha);
}

/* ================================ Sprites =================================
 * The interface exported is opaque and only uses void pointers, so that a
 * different implementation of the framebuffer.c not using SDL can retain
 * the same interface with load81.c. */
#define SPRITE_MT "l81.sprite_mt"

void spriteBlit(frameBuffer *fb, void *sprite, int x, int y, int angle, int aa) {
    SDL_Surface *s = sprite;
    if (s == NULL) return;
    if (angle) s = rotozoomSurface(s,angle,1,aa);
    SDL_Rect dst = {x, fb->height-1-y - s->h, s->w, s->h};
    SDL_BlitSurface(s, NULL, fb->screen, &dst);
    if (angle) SDL_FreeSurface(s);
}

/* Load sprite.  Return surface pointer and object on top of stack */
void *spriteLoad(lua_State *L, const char *filename) {
    SDL_Surface **pps;

    /* check if image was already loaded and cached */
    lua_getglobal(L, "sprites");
    lua_getfield(L, -1, filename);
    if (lua_isnil(L, -1)) {
        /* load image into surface */
        SDL_Surface *ps = IMG_Load(filename);
        if (ps == NULL) {   
            luaL_error(L, "failed to load sprite %s", filename);
            return NULL;
        }

        /* box the surface pointer in a userdata */
        pps = (SDL_Surface **)lua_newuserdata(L, sizeof(SDL_Surface *));
        *pps = ps;

        /* set sprite metatable */
        luaL_getmetatable(L, SPRITE_MT);
        lua_setmetatable(L, -2);

        /* cache loaded surface in sprite table */
        lua_pushvalue(L, -1); 
        lua_setfield(L, -4, filename);
    } else {
        /* unbox surface pointer */
        pps = (SDL_Surface **)luaL_checkudata(L, -1, SPRITE_MT);
    }
    return *pps;
}

int spriteGC(lua_State *L) {
    SDL_Surface **pps = (SDL_Surface **)luaL_checkudata(L, 1, SPRITE_MT);
    if (pps) SDL_FreeSurface(*pps);
    return 0;
}

int spriteGetHeight(lua_State *L) {
    SDL_Surface **pps = (SDL_Surface **)luaL_checkudata(L, 1, SPRITE_MT);
    lua_pushnumber(L, (*pps)->h);
    return 1;
}

int spriteGetWidth(lua_State *L) {
    SDL_Surface **pps = (SDL_Surface **)luaL_checkudata(L, 1, SPRITE_MT);
    lua_pushnumber(L, (*pps)->w);
    return 1;
}

static const struct luaL_Reg sprite_m[] = {
    { "__gc",      spriteGC        },
    { "getHeight", spriteGetHeight },
    { "getWidth",  spriteGetWidth  },
    { NULL,        NULL            }
};

void initSpriteEngine(lua_State *L) {
    luaL_newmetatable(L, SPRITE_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_register(L, NULL, sprite_m);
}
