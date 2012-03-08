#include <math.h>
#include <SDL.h>
#include "load81.h"
#include "drawing.h"

/* ============================= Frame buffer ============================== */

frameBuffer *createFrameBuffer(int width, int height) {
    frameBuffer *fb = malloc(sizeof(*fb));

    fb->p = malloc(width*height*3);
    fb->width = width;
    fb->height = height;
    return fb;
}

SDL_Surface *sdlInit(int width, int height, int fullscreen) {
    int flags = SDL_SWSURFACE;
    SDL_Surface *screen;

    if (fullscreen) flags |= SDL_FULLSCREEN;
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        fprintf(stderr, "SDL Init error: %s\n", SDL_GetError());
        return NULL;
    }
    atexit(SDL_Quit);
    screen = SDL_SetVideoMode(width,height,24,flags);
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

void sdlShowRgb(SDL_Surface *screen, frameBuffer *fb)
{
    unsigned char *s, *p = fb->p;
    int y,x;

    for (y = 0; y < fb->height; y++) {
        s = screen->pixels+y*(screen->pitch);
        for (x = 0; x < fb->width; x++) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
            s[0] = p[2];
            s[1] = p[1];
            s[2] = p[0];
#else
	    memcpy(s, p, 3);
#endif
            s += 3;
            p += 3;
        }
    }
    SDL_UpdateRect(screen, 0, 0, fb->width-1, fb->height-1);
}

/* ========================== Drawing primitives ============================ */

void setPixelWithAlpha(frameBuffer *fb, int x, int y, int r, int g, int b, float alpha) {
    unsigned char *p;

    y = fb->height-1-y; /* y=0 is bottom border of the screen */
    p = fb->p+y*fb->width*3+x*3;

    if (x < 0 || x >= fb->width || y < 0 || y >= fb->height) return;
    p[0] = (alpha*r)+((1-alpha)*p[0]);
    p[1] = (alpha*g)+((1-alpha)*p[1]);
    p[2] = (alpha*b)+((1-alpha)*p[2]);
}

void drawHline(frameBuffer *fb, int x1, int x2, int y, int r, int g, int b, float alpha) {
    int aux, x;

    if (x1 > x2) {
        aux = x1;
        x1 = x2;
        x2 = aux;
    }
    for (x = x1; x <= x2; x++)
        setPixelWithAlpha(fb,x,y,r,g,b,alpha);
}

void drawEllipse(frameBuffer *fb, int xc, int yc, int radx, int rady, int r, int g, int b, float alpha) {
    int x1, x2, y;
    float xshift;

    for (y=yc-rady; y<=yc+rady; y++) {
        xshift = sqrt((rady*rady) - ((y - yc)*(y - yc)))*((float)radx/rady);
        x1 = round(xc-xshift);
        x2 = round(xc+xshift);
        drawHline(fb,x1,x2,y,r,g,b,alpha);
    }
}

void drawBox(frameBuffer *fb, int x1, int y1, int x2, int y2, int r, int g, int b, float alpha) {
    int x, y;

    for (x = x1; x <= x2; x++ ) {
        for (y = y1; y <= y2; y++) {
            setPixelWithAlpha(fb,x,y,r,g,b,alpha);
        }
    }
}

void drawTriangle(frameBuffer *fb, int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b, float alpha) {
    int swap, t;
    struct {
        float x, y;
    } A, B, C, E, S;
    float dx1,dx2,dx3;

    A.x = x1;
    A.y = y1;
    B.x = x2;
    B.y = y2;
    C.x = x3;
    C.y = y3;

    /* For this algorithm to work we need to sort A, B, C by 'y' */
    do {
        swap = 0;
        if (A.y > B.y) {
            t = A.y; A.y = B.y; B.y = t;
            t = A.x; A.x = B.x; B.x = t;
            swap++;
        }
        if (B.y > C.y) {
            t = B.y; B.y = C.y; C.y = t;
            t = B.x; B.x = C.x; C.x = t;
            swap++;
        }
    } while(swap);

    if (B.y-A.y > 0) dx1=(B.x-A.x)/(B.y-A.y); else dx1=B.x - A.x;
    if (C.y-A.y > 0) dx2=(C.x-A.x)/(C.y-A.y); else dx2=0;
    if (C.y-B.y > 0) dx3=(C.x-B.x)/(C.y-B.y); else dx3=0;

    S=E=A;
    if(dx1 > dx2) {
        for(;S.y<=B.y;S.y++,E.y++,S.x+=dx2,E.x+=dx1)
            drawHline(fb,S.x,E.x,S.y,r,g,b,alpha);
        E=B;
        E.y+=1;
        for(;S.y<=C.y;S.y++,E.y++,S.x+=dx2,E.x+=dx3)
            drawHline(fb,S.x,E.x,S.y,r,g,b,alpha);
    } else {
        for(;S.y<=B.y;S.y++,E.y++,S.x+=dx1,E.x+=dx2)
            drawHline(fb,S.x,E.x,S.y,r,g,b,alpha);
        S=B;
        S.y+=1;
        for(;S.y<=C.y;S.y++,E.y++,S.x+=dx3,E.x+=dx2)
            drawHline(fb,S.x,E.x,S.y,r,g,b,alpha);
    }
}


/* Bresenham algorithm */
void drawLine(frameBuffer *fb, int x1, int y1, int x2, int y2, int r, int g, int b, float alpha) {
    int dx = abs(x2-x1);
    int dy = abs(y2-y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx-dy, e2;

    while(1) {
        setPixelWithAlpha(fb,x1,y1,r,g,b,alpha);
        if (x1 == x2 && y1 == y2) break;
        e2 = err*2;
        if (e2 > -dy) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y1 += sy;
        }
    }
}

/* ============================= Bitmap font =============================== */
void bfLoadFont(char **c) {
    /* Set all the entries to NULL. */
    memset(c,0,sizeof(unsigned char*)*256);
    /* Now populate the entries we have in our bitmap font description. */
    #include "bitfont.dat"
}

void bfWriteChar(frameBuffer *fb, int xp, int yp, int c, int r, int g, int b, float alpha) {
    int x,y;
    unsigned char *bitmap = l81.font[c&0xff];

    if (!bitmap) bitmap = l81.font['?'];
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            int byte = (y*16+x)/8;
            int bit = x%8;
            int set = bitmap[byte] & (0x80>>bit);

            if (set) setPixelWithAlpha(fb,xp+x,yp-y+15,r,g,b,alpha);
        }
    }
}

void bfWriteString(frameBuffer *fb, int xp, int yp, const char *s, int len, int r, int g, int b, float alpha) {
    int i;

    for (i = 0; i < len; i++)
        bfWriteChar(fb,xp-((16-FONT_KERNING)/2)+i*FONT_KERNING,yp,
                    s[i],r,g,b,alpha);
}
