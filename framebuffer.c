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
