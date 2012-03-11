#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_framerate.h>

#define FONT_WIDTH 16
#define FONT_HEIGHT 16
#define FONT_KERNING 10

typedef struct frameBuffer {
    int width;
    int height;
    SDL_Surface *screen;
    FPSmanager fps_mgr;
} frameBuffer;

SDL_Surface *sdlInit(int width, int height, int fullscreen);
frameBuffer *createFrameBuffer(int width, int height, int fullscreen);
void setPixelWithAlpha(frameBuffer *fb, int x, int y, int r, int g, int b, int alpha);
void fillBackground(frameBuffer *fb, int r, int g, int b);
void drawHline(frameBuffer *fb, int x1, int x2, int y, int r, int g, int b, int alpha);
void drawEllipse(frameBuffer *fb, int xc, int yc, int radx, int rady, int r, int g, int b, int alpha);
void drawBox(frameBuffer *fb, int x1, int y1, int x2, int y2, int r, int g, int b, int alpha);
void drawTriangle(frameBuffer *fb, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, int alpha);
void drawLine(frameBuffer *fb, int x1, int y1, int x2, int y2, int r, int g, int b, int alpha);
void bfLoadFont(char **c);
void bfWriteChar(frameBuffer *fb, int xp, int yp, int c, int r, int g, int b, int alpha);
void bfWriteString(frameBuffer *fb, int xp, int yp, const char *s, int len, int r, int g, int b, int alpha);

#endif /* FRAMEBUFFER_H */
