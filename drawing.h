#ifndef __LOAD81_DRAWING__
#define __LOAD81_DRAWING__

#include <math.h>
#include <SDL.h>

#define FONT_KERNING 10

typedef struct frameBuffer {
    unsigned char *p;
    int width;
    int height;
} frameBuffer;

frameBuffer *createFrameBuffer(int width, int height);
SDL_Surface *sdlInit(int width, int height, int fullscreen);
void sdlShowRgb(SDL_Surface *screen, frameBuffer *fb);
void setPixelWithAlpha(frameBuffer *fb, int x, int y, int r, int g, int b, float alpha);
void drawHline(frameBuffer *fb, int x1, int x2, int y, int r, int g, int b, float alpha);
void drawEllipse(frameBuffer *fb, int xc, int yc, int radx, int rady, int r, int g, int b, float alpha);
void drawBox(frameBuffer *fb, int x1, int y1, int x2, int y2, int r, int g, int b, float alpha);
void drawTriangle(frameBuffer *fb, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, float alpha);
void drawLine(frameBuffer *fb, int x1, int y1, int x2, int y2, int r, int g, int b, float alpha);
void bfLoadFont(char **c);
void bfWriteChar(frameBuffer *fb, int xp, int yp, int c, int r, int g, int b, float alpha);
void bfWriteString(frameBuffer *fb, int xp, int yp, const char *s, int len, int r, int g, int b, float alpha);


#endif
