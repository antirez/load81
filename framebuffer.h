#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_framerate.h>
#include <SDL_image.h>
#include <SDL_rotozoom.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define FONT_WIDTH 16
#define FONT_HEIGHT 16
#define FONT_KERNING 10

typedef struct frameBuffer {
    int width;
    int height;
    SDL_Surface *screen;
    FPSmanager fps_mgr;
} frameBuffer;

/* Frame buffer */
frameBuffer *createFrameBuffer(int width, int height, int bpp, int fullscreen);

/* Drawing primitives */
void setPixelWithAlpha(frameBuffer *fb, int x, int y, int r, int g, int b, int alpha);
void fillBackground(frameBuffer *fb, int r, int g, int b);
void drawHline(frameBuffer *fb, int x1, int x2, int y, int r, int g, int b, int alpha);
void drawEllipse(frameBuffer *fb, int xc, int yc, int radx, int rady, int r, int g, int b, int alpha, char filled);
void drawBox(frameBuffer *fb, int x1, int y1, int x2, int y2, int r, int g, int b, int alpha, char filled);
void drawTriangle(frameBuffer *fb, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int alpha, char filled);
void drawLine(frameBuffer *fb, int x1, int y1, int x2, int y2, int r, int g, int b, int alpha);
void drawPolygon(frameBuffer *fb, Sint16* xv, Sint16* yv, int n, int r, int g, int b, int alpha, char filled);

/* Bitmap font */
void bfLoadFont(char **c);
void bfWriteChar(frameBuffer *fb, int xp, int yp, int c, int r, int g, int b, int alpha);
void bfWriteString(frameBuffer *fb, int xp, int yp, const char *s, int len, int r, int g, int b, int alpha);

/* Sprites */
void spriteBlit(frameBuffer *fb, void *sprite, int x, int y, int angle, int aa);
void *spriteLoad(lua_State *L, const char *filename);
void initSpriteEngine(lua_State *L);

#endif /* FRAMEBUFFER_H */
