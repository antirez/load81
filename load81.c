/******************************************************************************
* Copyright (C) 2012 Salvatore Sanfilippo.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <ctype.h>

#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_framerate.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#define NOTUSED(V) ((void) V)

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600
#define FONT_WIDTH 16
#define FONT_HEIGHT 16
#define FONT_KERNING 10

#define POWEROFF_BUTTON_X   (l81.width-18)
#define POWEROFF_BUTTON_Y   18
#define SAVE_BUTTON_X       (l81.width-E.margin_right-13)
#define SAVE_BUTTON_Y       (l81.height-16)

#define EDITOR_FPS 30

/* ============================== Portable sleep ============================ */

#ifdef WIN32
#include <windows.h>
#define sleep_milliseconds(x) Sleep(x)
#else
#include <unistd.h>
#define sleep_milliseconds(x) usleep((x)*1000)
#endif

/* ============================= Data structures ============================ */

typedef struct frameBuffer {
    int width;
    int height;
    SDL_Surface *screen;
} frameBuffer;

struct globalConfig {
    /* Runtime */
    int width;
    int height;
    int r,g,b;
    int alpha;
    int fps;
    long long start_ms;
    long long epoch;
    FPSmanager fps_mgr;
    frameBuffer *fb;
    lua_State *L;
    unsigned char *font[256];
    char *filename;
    char *luaerr;
    int luaerrline;
    /* Command line switches */
    int opt_show_fps;
    int opt_full_screen;
} l81;

typedef struct erow {
    int size;
    char *chars;
} erow;

typedef struct keyState {
    char translation;
    int counter;
} keyState;

#define KEY_MAX 512 /* Latest key is excluded */
struct editorConfig {
    int cx,cy;  /* Cursor x and y position in characters */
    unsigned char cblink; /* Show cursor if (cblink & 0x80) == 0 */
    int screenrows; /* Number of rows that we can show */
    int screencols; /* Number of cols that we can show */
    int margin_top, margin_bottom, margin_left, margin_right;
    int rowoff;     /* Row offset on screen */
    int coloff;     /* Column offset on screen */
    int numrows;    /* Number of rows */
    erow *row;      /* Rows */
    time_t lastevent;   /* Last event time, so we can go standby */
    keyState key[KEY_MAX];   /* Remember if a key is pressed / repeated. */
    int dirty;      /* File modified but not saved. */
} E;

/* =========================== Utility functions ============================ */

/* Return the UNIX time in microseconds */
long long ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long)tv.tv_sec)*1000000;
    ust += tv.tv_usec;
    return ust;
}

/* Return the UNIX time in milliseconds */
long long mstime(void) {
    return ustime()/1000;
}

/* ============================= Frame buffer ============================== */
SDL_Surface *sdlInit(int width, int height, int fullscreen) {
    int flags = SDL_SWSURFACE;
    SDL_Surface *screen;

    if (fullscreen) flags |= SDL_FULLSCREEN;
    if (SDL_Init(SDL_INIT_VIDEO) == -1) {
        fprintf(stderr, "SDL Init error: %s\n", SDL_GetError());
        return NULL;
    }
    atexit(SDL_Quit);
    screen = SDL_SetVideoMode(width,height,0,flags);
    if (!screen) {
        fprintf(stderr, "Can't set the video mode: %s\n", SDL_GetError());
        return NULL;
    }
    /* Unicode support makes dealing with text input in SDL much simpler as
     * keys are translated into characters with automatic support for modifiers
     * (for instance shift modifier to print capital letters and symbols). */
    SDL_EnableUNICODE(SDL_ENABLE);
    SDL_initFramerate(&l81.fps_mgr);
    return screen;
}

frameBuffer *createFrameBuffer(int width, int height) {
    frameBuffer *fb = malloc(sizeof(*fb));

    fb->width = width;
    fb->height = height;
    fb->screen = sdlInit(width,height,l81.opt_full_screen);
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

void bfWriteString(frameBuffer *fb, int xp, int yp, const char *s, int len, int r, int g, int b, int alpha) {
    int i;

    for (i = 0; i < len; i++)
        bfWriteChar(fb,xp-((16-FONT_KERNING)/2)+i*FONT_KERNING,yp,
                    s[i],r,g,b,alpha);
}

/* ========================= Lua helper functions ========================== */

/* Set a Lua global to the specified number. */
void setNumber(char *name, lua_Number n) {
    lua_pushnumber(l81.L,n);
    lua_setglobal(l81.L,name);
}

/* Get a Lua global containing a number. */
lua_Number getNumber(char *name) {
    lua_Number n;

    lua_getglobal(l81.L,name);
    n = lua_tonumber(l81.L,-1);
    lua_pop(l81.L,1);
    return n;
}

/* Set a Lua global table field to the value on the top of the Lua stack. */
void setTableField(char *name, char *field) {
    lua_getglobal(l81.L,name);          /* Stack: val table */
    /* Create the table if needed */
    if (lua_isnil(l81.L,-1)) {
        lua_pop(l81.L,1);               /* Stack: val */
        lua_newtable(l81.L);            /* Stack: val table */
        lua_setglobal(l81.L,name);      /* Stack: val */
        lua_getglobal(l81.L,name);      /* Stack: val table */
    }
    /* Set the field */
    if (lua_istable(l81.L,-1)) {
        lua_pushstring(l81.L,field);    /* Stack: val table field */
        lua_pushvalue(l81.L,-3);        /* Stack: val table field val */
        lua_settable(l81.L,-3);         /* Stack: val table */
    }
    lua_pop(l81.L,2);                   /* Stack: (empty) */
}

void setTableFieldNumber(char *name, char *field, lua_Number n) {
    lua_pushnumber(l81.L,n);
    setTableField(name,field);
}

void setTableFieldString(char *name, char *field, char *s) {
    lua_pushstring(l81.L,s);
    setTableField(name,field);
}

/* Set the error string and the error line number. */
void programError(const char *e) {
    free(l81.luaerr);
    l81.luaerr = strdup(e);
    e = strstr(l81.luaerr,":");
    if (e)
        l81.luaerrline = atoi(e+1)-1;
}

/* ============================= Lua bindings ============================== */
int fillBinding(lua_State *L) {
    l81.r = lua_tonumber(L,-4);
    l81.g = lua_tonumber(L,-3);
    l81.b = lua_tonumber(L,-2);
    l81.alpha = lua_tonumber(L,-1) * 255;
    if (l81.r < 0) l81.r = 0;
    if (l81.r > 255) l81.r = 255;
    if (l81.g < 0) l81.g = 0;
    if (l81.g > 255) l81.g = 255;
    if (l81.b < 0) l81.b = 0;
    if (l81.b > 255) l81.b = 255;
    if (l81.alpha < 0) l81.alpha = 0;
    if (l81.alpha > 255) l81.alpha = 255;
    return 0;
}

int rectBinding(lua_State *L) {
    int x,y,w,h;

    x = lua_tonumber(L,-4);
    y = lua_tonumber(L,-3);
    w = lua_tonumber(L,-2);
    h = lua_tonumber(L,-1);
    drawBox(l81.fb,x,y,x+(w-1),y+(h-1),l81.r,l81.g,l81.b,l81.alpha);
    return 0;
}

int ellipseBinding(lua_State *L) {
    int x,y,rx,ry;

    x = lua_tonumber(L,-4);
    y = lua_tonumber(L,-3);
    rx = lua_tonumber(L,-2);
    ry = lua_tonumber(L,-1);
    drawEllipse(l81.fb,x,y,rx,ry,l81.r,l81.g,l81.b,l81.alpha);
    return 0;
}

int triangleBinding(lua_State *L) {
    int x1,y1,x2,y2,x3,y3;

    x1 = lua_tonumber(L,-6);
    y1 = lua_tonumber(L,-5);
    x2 = lua_tonumber(L,-4);
    y2 = lua_tonumber(L,-3);
    x3 = lua_tonumber(L,-2);
    y3 = lua_tonumber(L,-1);
    drawTriangle(l81.fb,x1,y1,x2,y2,x3,y3,l81.r,l81.g,l81.b,l81.alpha);
    return 0;
}

int lineBinding(lua_State *L) {
    int x1,y1,x2,y2;

    x1 = lua_tonumber(L,-4);
    y1 = lua_tonumber(L,-3);
    x2 = lua_tonumber(L,-2);
    y2 = lua_tonumber(L,-1);
    drawLine(l81.fb,x1,y1,x2,y2,l81.r,l81.g,l81.b,l81.alpha);
    return 0;
}

int textBinding(lua_State *L) {
    int x,y;
    const char *s;
    size_t len;

    x = lua_tonumber(L,-3);
    y = lua_tonumber(L,-2);
    s = lua_tolstring(L,-1,&len);
    if (!s) return 0;
    bfWriteString(l81.fb,x,y,s,len,l81.r,l81.g,l81.b,l81.alpha);
    return 0;
}

int setFPSBinding(lua_State *L) {
    l81.fps = lua_tonumber(L,-1);

    if (l81.fps <= 0) l81.fps = 1;
    SDL_setFramerate(&l81.fps_mgr,l81.fps);
    return 0;
}

int backgroundBinding(lua_State *L) {
    int r,g,b;

    r = lua_tonumber(L,-3);
    g = lua_tonumber(L,-2);
    b = lua_tonumber(L,-1);
    fillBackground(l81.fb,r,g,b);
    return 0;
}

int getpixelBinding(lua_State *L) {
    uint32_t pixel;
    uint8_t r, g, b;
    int x, y;

    x = lua_tonumber(L,-2);
    y = l81.fb->height - 1 - lua_tonumber(L,-1);

    SDL_LockSurface(l81.fb->screen);
    if (x < 0 || x >= l81.fb->width || y < 0 || y >= l81.fb->height) {
        pixel = 0;
    } else {
        int bpp;
        unsigned char *p;

        bpp = l81.fb->screen->format->BytesPerPixel;
        p = ((unsigned char*) l81.fb->screen->pixels)+
                             (y*l81.fb->screen->pitch)+(x*bpp);
        switch(bpp) {
        case 1: pixel = *p; break;
        case 2: pixel = *(uint16_t *)p; break;
        case 3:
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
            pixel = p[0]|p[1]<<8|p[2]<<16;
#else
            pixel = p[2]|p[1]<<8|p[0]<<16;
#endif
        case 4: pixel = *(uint32_t*)p; break;
        default: return 0; break;
        }
    }
    SDL_GetRGB(pixel,l81.fb->screen->format,&r,&g,&b);
    SDL_UnlockSurface(l81.fb->screen);
    /* Return the pixel as three values: r, g, b. */
    lua_pushnumber(L,r);
    lua_pushnumber(L,g);
    lua_pushnumber(L,b);
    return 3;
}

/* ========================== Events processing ============================= */

void setup(void) {
    lua_getglobal(l81.L,"setup");
    if (!lua_isnil(l81.L,-1)) {
        if (lua_pcall(l81.L,0,0,0)) {
            programError(lua_tostring(l81.L, -1));
        }
    } else {
        lua_pop(l81.L,1);
    }
}

void draw(void) {
    lua_getglobal(l81.L,"draw");
    if (!lua_isnil(l81.L,-1)) {
        if (lua_pcall(l81.L,0,0,0)) {
            programError(lua_tostring(l81.L, -1));
        }
    } else {
        lua_pop(l81.L,1);
    }
}

/* Update the keyboard.pressed and mouse.pressed Lua table. */
void updatePressedState(char *object, char *keyname, int pressed) {
    lua_getglobal(l81.L,object);         /* $keyboard */
    lua_pushstring(l81.L,"pressed");     /* $keyboard, "pressed" */
    lua_gettable(l81.L,-2);              /* $keyboard, $pressed */
    lua_pushstring(l81.L,keyname);       /* $keyboard, $pressed, "keyname" */
    if (pressed) {
        lua_pushboolean(l81.L,1);        /* $k, $pressed, "keyname", true */
    } else {
        lua_pushnil(l81.L);              /* $k, $pressed, "keyname", nil */
    }
    lua_settable(l81.L,-3);              /* $k, $pressed */
    lua_pop(l81.L,2);
}

void keyboardEvent(SDL_KeyboardEvent *key, int down) {
    char *keyname = SDL_GetKeyName(key->keysym.sym);

    setTableFieldString("keyboard","state",down ? "down" : "up");
    setTableFieldString("keyboard","key",keyname);
    updatePressedState("keyboard",keyname,down);
}

void mouseMovedEvent(int x, int y, int xrel, int yrel) {
    setTableFieldNumber("mouse","x",x);
    setTableFieldNumber("mouse","y",l81.height-1-y);
    setTableFieldNumber("mouse","xrel",xrel);
    setTableFieldNumber("mouse","yrel",-yrel);
}

void mouseButtonEvent(int button, int pressed) {
    char buttonname[32];
    
    snprintf(buttonname,sizeof(buttonname),"%d",button);
    updatePressedState("mouse",buttonname,pressed);
}

void resetEvents(void) {
    setTableFieldString("keyboard","state","none");
    setTableFieldString("keyboard","key","");
}

void showFPS(void) {
    int elapsed_ms = mstime()-l81.start_ms;
    char buf[64];

    if (!elapsed_ms) return;
    snprintf(buf,sizeof(buf),"FPS: %.2f",(float)(l81.epoch*1000)/elapsed_ms);
    drawBox(l81.fb,0,0,100,20,0,0,0,255);
    bfWriteString(l81.fb,0,0,buf,strlen(buf),128,128,128,255);
}

int processSdlEvents(void) {
    SDL_Event event;

    resetEvents();
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
        case SDL_KEYDOWN:
            switch(event.key.keysym.sym) {
            case SDLK_ESCAPE:
                return 1;
                break;
            default:
                keyboardEvent(&event.key,1);
                break;
            }
            break;
        case SDL_KEYUP:
            keyboardEvent(&event.key,0);
            break;
        case SDL_MOUSEMOTION:
            mouseMovedEvent(event.motion.x,event.motion.y,
                       event.motion.xrel,event.motion.yrel);
            break;
        case SDL_MOUSEBUTTONDOWN:
            mouseButtonEvent(event.button.button,1);
            break;
        case SDL_MOUSEBUTTONUP:
            mouseButtonEvent(event.button.button,0);
            break;
        case SDL_QUIT:
            exit(0);
            break;
        }
        /* If the next event to process is of type KEYUP or
         * MOUSEBUTTONUP we want to stop processing here, so that
         * a fast up/down event be noticed by Lua. */
        if (SDL_PeepEvents(&event,1,SDL_PEEKEVENT,SDL_ALLEVENTS)) {
            if (event.type == SDL_KEYUP ||
                event.type == SDL_MOUSEBUTTONUP)
                break; /* Go to lua before processing more. */
        }
    }

    /* Call the setup function, only the first time. */
    if (l81.epoch == 0) setup();
    /* Call the draw function at every iteration.  */
    draw();
    l81.epoch++;
    /* Refresh the screen */
    if (l81.opt_show_fps) showFPS();
    SDL_Flip(l81.fb->screen);
    /* Wait some time if the frame was produced in less than 1/FPS seconds. */
    SDL_framerateDelay(&l81.fps_mgr);
    /* Stop execution on error */
    return l81.luaerr != NULL;
}

/* ======================= Editor rows implementation ======================= */

/* Insert a row at the specified position, shifting the other rows on the bottom
 * if required. */
void editorInsertRow(int at, char *s) {
    if (at > E.numrows) return;
    E.row = realloc(E.row,sizeof(erow)*(E.numrows+1));
    if (at != E.numrows)
        memmove(E.row+at+1,E.row+at,sizeof(E.row[0])*(E.numrows-at));
    E.row[at].size = strlen(s);
    E.row[at].chars = strdup(s);
    E.numrows++;
    E.dirty++;
}

/* Remove the row at the specified position, shifting the remainign on the
 * top. */
void editorDelRow(int at) {
    if (at >= E.numrows) return;
    memmove(E.row+at,E.row+at+1,sizeof(E.row[0])*(E.numrows-at-1));
    E.numrows--;
    E.dirty++;
}

/* Turn the editor rows into a single heap-allocated string.
 * Returns the pointer to the heap-allocated string and populate the
 * integer pointed by 'buflen' with the size of the string, escluding
 * the final nulterm. */
char *editorRowsToString(int *buflen) {
    char *buf = NULL, *p;
    int totlen = 0;
    int j;

    /* Compute count of bytes */
    for (j = 0; j < E.numrows; j++)
        totlen += E.row[j].size+1; /* +1 is for "\n" at end of every row */
    *buflen = totlen;
    totlen++; /* Also make space for nulterm */

    p = buf = malloc(totlen);
    for (j = 0; j < E.numrows; j++) {
        memcpy(p,E.row[j].chars,E.row[j].size);
        p += E.row[j].size;
        *p = '\n';
        p++;
    }
    *p = '\0';
    return buf;
}

/* Insert a character at the specified position in a row, moving the remaining
 * chars on the right if needed. */
void editorRowInsertChar(erow *row, int at, int c) {
    if (at > row->size) {
        /* Pad the string with spaces if the insert location is outside the
         * current length by more than a single character. */
        int padlen = at-row->size;
        /* In the next line +2 means: new char and null term. */
        row->chars = realloc(row->chars,row->size+padlen+2);
        memset(row->chars+row->size,' ',padlen);
        row->chars[row->size+padlen+1] = '\0';
        row->size += padlen+1;
    } else {
        /* If we are in the middle of the string just make space for 1 new
         * char plus the (already existing) null term. */
        row->chars = realloc(row->chars,row->size+2);
        memmove(row->chars+at+1,row->chars+at,row->size-at+1);
        row->size++;
    }
    row->chars[at] = c;
    E.dirty++;
}

/* Append the string 's' at the end of a row */
void editorRowAppendString(erow *row, char *s) {
    int l = strlen(s);

    row->chars = realloc(row->chars,row->size+l+1);
    memcpy(row->chars+row->size,s,l);
    row->size += l;
    row->chars[row->size] = '\0';
    E.dirty++;
}

void editorRowDelChar(erow *row, int at) {
    if (row->size <= at) return;
    memmove(row->chars+at,row->chars+at+1,row->size-at);
    row->size--;
    E.dirty++;
}

void editorInsertChar(int c) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    /* If the row where the cursor is currently located does not exist in our
     * logical representaion of the file, add enough empty rows as needed. */
    if (!row) {
        while(E.numrows <= filerow)
            editorInsertRow(E.numrows,"");
    }
    row = &E.row[filerow];
    editorRowInsertChar(row,filecol,c);
    if (E.cx == E.screencols-1)
        E.coloff++;
    else
        E.cx++;
    E.dirty++;
}

/* Inserting a newline is slightly complex as we have to handle inserting a
 * newline in the middle of a line, splitting the line as needed. */
void editorInsertNewline(void) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (!row) {
        if (filerow == E.numrows) {
            editorInsertRow(filerow,"");
            goto fixcursor;
        }
        return;
    }
    /* If the cursor is over the current line size, we want to conceptually
     * think it's just over the last character. */
    if (filecol >= row->size) filecol = row->size;
    if (filecol == 0) {
        editorInsertRow(filerow,"");
    } else {
        /* We are in the middle of a line. Split it between two rows. */
        editorInsertRow(filerow+1,row->chars+filecol);
        row = &E.row[filerow];
        row->chars[filecol] = '\0';
        row->size = filecol;
    }
fixcursor:
    if (E.cy == E.screenrows-1) {
        E.rowoff++;
    } else {
        E.cy++;
    }
    E.cx = 0;
    E.coloff = 0;
}

void editorDelChar() {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (!row || (filecol == 0 && filerow == 0)) return;
    if (filecol == 0) {
        /* Handle the case of column 0, we need to move the current line
         * on the right of the previous one. */
        filecol = E.row[filerow-1].size;
        editorRowAppendString(&E.row[filerow-1],row->chars);
        editorDelRow(filerow);
        if (E.cy == 0)
            E.rowoff--;
        else
            E.cy--;
        E.cx = filecol;
        if (E.cx >= E.screencols) {
            int shift = (E.screencols-E.cx)+1;
            E.cx -= shift;
            E.coloff += shift;
        }
    } else {
        editorRowDelChar(row,filecol-1);
        if (E.cx == 0 && E.coloff)
            E.coloff--;
        else
            E.cx--;
    }
    E.dirty++;
}

/* Load the specified program in the editor memory and returns 0 on success
 * or 1 on error. */
int editorOpen(char *filename) {
    FILE *fp;
    char line[1024];

    /* TODO: remove old program from rows. */
    fp = fopen(filename,"r");
    if (!fp) {
        perror("fopen loading program into editor");
        return 1;
    }
    while(fgets(line,sizeof(line),fp) != NULL) {
        int l = strlen(line);

        if (l && (line[l-1] == '\n' || line[l-1] == '\r'))
            line[l-1] = '\0';
        editorInsertRow(E.numrows,line);
    }
    fclose(fp);
    E.dirty = 0;
    return 0;
}

int editorSave(char *filename) {
    int len;
    char *buf = editorRowsToString(&len);
    FILE *fp;

    fp = fopen(filename,"w");
    if (!fp) {
        free(buf);
        return 1;
    }
    fwrite(buf,len,1,fp);
    fclose(fp);
    free(buf);
    return 0;
}

/* ============================= Editor drawing ============================= */

void editorDrawCursor(void) {
    int x = E.cx*FONT_KERNING;
    int y = l81.height-((E.cy+1)*FONT_HEIGHT);
    int charmargin = (FONT_WIDTH-FONT_KERNING)/2;

    x += E.margin_left;
    y -= E.margin_top;
    if (!(E.cblink & 0x80)) drawBox(l81.fb,x+charmargin,y,
                                x+charmargin+FONT_KERNING-1,y+FONT_HEIGHT-1,
                                165,165,255,128);
    E.cblink += 4;
}

#define LINE_TYPE_NORMAL 0
#define LINE_TYPE_COMMENT 1
#define LINE_TYPE_ERROR 2

int editorLineType(erow *row, int filerow) {
    char *p = row->chars;

    if (l81.luaerr && l81.luaerrline == filerow) return LINE_TYPE_ERROR;
    while(*p == ' ') p++;
    if (*p == '-' && *(p+1) == '-') return LINE_TYPE_COMMENT;
    return LINE_TYPE_NORMAL;
}

void editorDrawChars(void) {
    int y,x;
    erow *r;
    char buf[32];

    for (y = 0; y < E.screenrows; y++) {
        int chary, filerow = E.rowoff+y;

        if (filerow >= E.numrows) break;
        chary = l81.height-((y+1)*FONT_HEIGHT);
        chary -= E.margin_top;
        r = &E.row[filerow];

        snprintf(buf,sizeof(buf),"%d",filerow%1000);
        bfWriteString(l81.fb,0,chary,buf,strlen(buf),120,120,120,255);

        for (x = 0; x < E.screencols; x++) {
            int idx = x+E.coloff;
            int charx;
            int tr,tg,tb;
            int line_type = editorLineType(r,filerow);

            if (idx >= r->size) break;
            charx = x*FONT_KERNING;
            charx += E.margin_left;
            switch(line_type) {
            case LINE_TYPE_ERROR: tr = 255; tg = 100, tb = 100; break;
            case LINE_TYPE_COMMENT: tr = 180, tg = 180, tb = 0; break;
            default: tr = 165; tg = 165, tb = 255; break;
            }
            bfWriteChar(l81.fb,charx,chary,r->chars[idx],tr,tg,tb,255);
        }
    }
    if (l81.luaerr) {
        char *p = strchr(l81.luaerr,':');
        p = p ? p+1 : l81.luaerr;
        bfWriteString(l81.fb,E.margin_left,10,p,strlen(p),0,0,0,255);
    }
}

void editorDrawPowerOff(int x, int y) {
    drawEllipse(l81.fb,x,y,12,12,66,66,231,255);
    drawEllipse(l81.fb,x,y,7,7,165,165,255,255);
    drawBox(l81.fb,x-4,y,x+4,y+12,165,165,255,255);
    drawBox(l81.fb,x-2,y,x+2,y+14,66,66,231,255);
}

void editorDrawSaveIcon(int x, int y) {
    drawBox(l81.fb,x-12,y-12,x+12,y+12,66,66,231,255);
    drawBox(l81.fb,x-1,y+7,x+1,y+11,165,165,255,255);
    drawEllipse(l81.fb,x,y,4,4,165,165,255,255);
}

void editorDraw() {
    drawBox(l81.fb,0,0,l81.width-1,l81.height-1,165,165,255,255);
    drawBox(l81.fb,
            E.margin_left,
            E.margin_top,
            l81.width-1-E.margin_right,
            l81.height-1-E.margin_bottom,66,66,231,255);
    editorDrawChars();
    editorDrawCursor();
    /* Show buttons */
    editorDrawPowerOff(POWEROFF_BUTTON_X,POWEROFF_BUTTON_Y);
    if (E.dirty) editorDrawSaveIcon(SAVE_BUTTON_X,SAVE_BUTTON_Y);
    /* Show info about the current file */
    bfWriteString(l81.fb,E.margin_left,l81.height-E.margin_top+4,l81.filename,
        strlen(l81.filename), 255,255,255,255);
}

/* ========================= Editor events handling  ======================== */

/* As long as a key is pressed, we incremnet a counter in order to
 * implement first pression of key and key repeating.
 *
 * This function returns if the key was just pressed or if it is repeating. */
#define KEY_REPEAT_PERIOD 4
#define KEY_REPEAT_PERIOD_FAST 1
#define KEY_REPEAT_DELAY 16
int pressed_or_repeated(int counter) {
    int period;

    if (counter > KEY_REPEAT_DELAY+(KEY_REPEAT_PERIOD*3)) {
        period = KEY_REPEAT_PERIOD_FAST;
    } else {
        period = KEY_REPEAT_PERIOD;
    }
    if (counter > 1 && counter < KEY_REPEAT_DELAY) return 0;
    return ((counter+period-1) % period) == 0;
}

void editorMouseClicked(int x, int y, int button) {
    if (abs(x-POWEROFF_BUTTON_X) < 15 && abs(y-POWEROFF_BUTTON_Y) < 15 &&
        button == 1)
    {
        exit(1);
    } else if (abs(x-SAVE_BUTTON_X) < 15 && abs(y-SAVE_BUTTON_Y) < 15 &&
               button == 1) {
        if (editorSave(l81.filename) == 0) E.dirty = 0;
    }
}

void editorMoveCursor(int key) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    int rowlen;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    switch(key) {
    case SDLK_LEFT:
        if (E.cx == 0) {
            if (E.coloff) E.coloff--;
        } else {
            E.cx -= 1;
        }
        break;
    case SDLK_RIGHT:
        if (row && filecol < row->size) {
            if (E.cx == E.screencols-1) {
                E.coloff++;
            } else {
                E.cx += 1;
            }
        }
        break;
    case SDLK_UP:
        if (E.cy == 0) {
            if (E.rowoff) E.rowoff--;
        } else {
            E.cy -= 1;
        }
        break;
    case SDLK_DOWN:
        if (filerow < E.numrows) {
            if (E.cy == E.screenrows-1) {
                E.rowoff++;
            } else {
                E.cy += 1;
            }
        }
        break;
    }
    /* Fix cx if the current line has not enough chars. */
    filerow = E.rowoff+E.cy;
    filecol = E.coloff+E.cx;
    row = (filerow >= E.numrows) ? NULL : &E.row[filerow];
    rowlen = row ? row->size : 0;
    if (filecol > rowlen) {
        E.cx -= filecol-rowlen;
        if (E.cx < 0) {
            E.coloff += E.cx;
            E.cx = 0;
        }
    }
}

int editorEvents(void) {
    SDL_Event event;
    int j, ksym;
    time_t idletime;

    /* Sleep 0.25 seconds if no body is pressing any key for a few seconds.
     * This way we can save tons of energy when in editing mode and
     * the user is thinking or away from keyboard. */
    idletime = time(NULL)-E.lastevent;
    if (idletime > 5) {
        sleep_milliseconds((idletime < 60) ? 50 : 250);
        E.cblink = 0;
    }

    while (SDL_PollEvent(&event)) {
        E.lastevent = time(NULL);
        switch(event.type) {
        /* Key pressed */
        case SDL_KEYDOWN:
            ksym = event.key.keysym.sym;
            switch(ksym) {
            case SDLK_ESCAPE:
                return 1;
                break;
            default:
                if (ksym >= 0 && ksym < KEY_MAX) {
                    E.key[ksym].counter = 1;
                    E.key[ksym].translation = (event.key.keysym.unicode & 0xff);
                }
                break;
            }
            break;

        /* Key released */
        case SDL_KEYUP:
            ksym = event.key.keysym.sym;
            if (ksym >= 0 && ksym < KEY_MAX) E.key[ksym].counter = 0;
            break;
        /* Mouse click */
        case SDL_MOUSEBUTTONDOWN:
            editorMouseClicked(event.motion.x, l81.height-1-event.motion.y,
                               event.button.button);
            break;
        case SDL_QUIT:
            exit(0);
            break;
        }
    }

    /* Convert events into actions */
    for (j = 0; j < KEY_MAX; j++) {
        int i;

        if (pressed_or_repeated(E.key[j].counter)) {
            E.lastevent = time(NULL);
            E.cblink = 0;
            switch(j) {
            case SDLK_LEFT:
            case SDLK_RIGHT:
            case SDLK_UP:
            case SDLK_DOWN:
                editorMoveCursor(j);
                break;
            case SDLK_BACKSPACE:
                editorDelChar();
                break;
            case SDLK_RETURN:
                editorInsertNewline();
                break;
            case SDLK_HOME:
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
            case SDLK_LCTRL:
            case SDLK_RCTRL:
            case SDLK_LALT:
            case SDLK_RALT:
            case SDLK_LMETA:
            case SDLK_RMETA:
                /* Ignored */
                break;
            case SDLK_TAB:
                for (i = 0; i < 4; i++)
                    editorInsertChar(' ');
                break;
            default:
                editorInsertChar(E.key[j].translation);
                break;
            }
        }
        if (E.key[j].counter) E.key[j].counter++; /* auto repeat counter */
    }

    /* Call the draw function at every iteration.  */
    editorDraw();
    /* Refresh the screen */
    SDL_Flip(l81.fb->screen);
    SDL_framerateDelay(&l81.fps_mgr);
    return 0;
}

/* =========================== Initialization ============================== */

void initConfig(void) {
    l81.width = DEFAULT_WIDTH;
    l81.height = DEFAULT_HEIGHT;
    l81.fps = 30;
    l81.r = 255;
    l81.g = l81.b = 0;
    l81.alpha = 255;
    l81.L = NULL;
    l81.luaerr = NULL;
    l81.luaerrline = 0;
    l81.opt_show_fps = 0;
    l81.opt_full_screen = 0;
    l81.filename = NULL;

    /* Load the bitmap font */
    bfLoadFont((char**)l81.font);
}

/* Load the editor program into Lua. Returns 0 on success, 1 on error. */
int loadProgram(void) {
    int buflen;
    char *buf = editorRowsToString(&buflen);

    if (luaL_loadbuffer(l81.L,buf,buflen,l81.filename)) {
        programError(lua_tostring(l81.L, -1));
        free(buf);
        return 1;
    }
    free(buf);
    if (lua_pcall(l81.L,0,0,0)) {
        programError(lua_tostring(l81.L, -1));
        return 1;
    }
    l81.luaerr = NULL;
    return 0;
}

void initEditor(void) {
    E.cx = 0;
    E.cy = 0;
    E.cblink = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.margin_top = E.margin_bottom = E.margin_left = E.margin_right = 30;
    E.screencols = (l81.width-E.margin_left-E.margin_right) / FONT_KERNING;
    E.screenrows = (l81.height-E.margin_top-E.margin_bottom) / FONT_HEIGHT;
    E.dirty = 0;
    memset(E.key,0,sizeof(E.key));
}

void initScreen(void) {
    l81.fb = createFrameBuffer(l81.width,l81.height);
}

void resetProgram(void) {
    char *initscript =
        "keyboard={}; keyboard['pressed']={};"
        "mouse={}; mouse['pressed']={};";

    l81.epoch = 0;
    if (l81.L) lua_close(l81.L);
    l81.L = lua_open();
    luaopen_base(l81.L);
    luaopen_table(l81.L);
    luaopen_string(l81.L);
    luaopen_math(l81.L);
    luaopen_debug(l81.L);
    setNumber("WIDTH",l81.width);
    setNumber("HEIGHT",l81.height);
    luaL_loadbuffer(l81.L,initscript,strlen(initscript),"initscript");
    lua_pcall(l81.L,0,0,0);

    /* Make sure that mouse parameters make sense even before the first
     * mouse event captured by SDL */
    setTableFieldNumber("mouse","x",0);
    setTableFieldNumber("mouse","y",0);
    setTableFieldNumber("mouse","xrel",0);
    setTableFieldNumber("mouse","yrel",0);

    /* Register API */
    lua_pushcfunction(l81.L,fillBinding);
    lua_setglobal(l81.L,"fill");
    lua_pushcfunction(l81.L,rectBinding);
    lua_setglobal(l81.L,"rect");
    lua_pushcfunction(l81.L,ellipseBinding);
    lua_setglobal(l81.L,"ellipse");
    lua_pushcfunction(l81.L,backgroundBinding);
    lua_setglobal(l81.L,"background");
    lua_pushcfunction(l81.L,triangleBinding);
    lua_setglobal(l81.L,"triangle");
    lua_pushcfunction(l81.L,lineBinding);
    lua_setglobal(l81.L,"line");
    lua_pushcfunction(l81.L,textBinding);
    lua_setglobal(l81.L,"text");
    lua_pushcfunction(l81.L,setFPSBinding);
    lua_setglobal(l81.L,"setFPS");
    lua_pushcfunction(l81.L,getpixelBinding);
    lua_setglobal(l81.L,"getpixel");

    /* Start with a black screen */
    fillBackground(l81.fb,0,0,0);
}

/* ================================= Main ================================== */

void showCliHelp(void) {
    fprintf(stderr, "Usage: load81 [options] program.lua\n"
           "  --width <pixels>       Set screen width\n"
           "  --height <pixels>      Set screen height\n"
           "  --full                 Enable full screen mode\n"
           "  --fps                  Show frames per second\n"
           "  --help                 Show this help screen\n"
           );
    exit(1);
}

void parseOptions(int argc, char **argv) {
    int j;

    for (j = 1; j < argc; j++) {
        char *arg = argv[j];
        int lastarg = j == argc-1;

        if (!strcasecmp(arg,"--fps")) {
            l81.opt_show_fps = 1;
        } else if (!strcasecmp(arg,"--full")) {
            l81.opt_full_screen = 1;
        } else if (!strcasecmp(arg,"--width") && !lastarg) {
            l81.width = atoi(argv[++j]);
        } else if (!strcasecmp(arg,"--height") && !lastarg) {
            l81.height = atoi(argv[++j]);
        } else if (!strcasecmp(arg,"--help")) {
            showCliHelp();
        } else {
            if (l81.filename == NULL && arg[0] != '-') {
                l81.filename = arg;
            } else {
                fprintf(stderr,
                    "Unrecognized option or missing argument: %s\n\n", arg);
                showCliHelp();
            }
        }
    }
    if (l81.filename == NULL) {
        fprintf(stderr,"No Lua program filename specified.\n\n");
        showCliHelp();
    }
}

int main(int argc, char **argv) {
    NOTUSED(argc);
    NOTUSED(argv);

    initConfig();
    parseOptions(argc,argv);
    initEditor();
    initScreen();
    editorOpen(l81.filename);
    while(1) {
        resetProgram();
        loadProgram();
        if (l81.luaerr == NULL) {
            SDL_setFramerate(&l81.fps_mgr,l81.fps);
            l81.start_ms = mstime();
            while(!processSdlEvents());
            if (E.dirty && editorSave(l81.filename) == 0) E.dirty = 0;
        }
        E.lastevent = time(NULL);
        SDL_setFramerate(&l81.fps_mgr,EDITOR_FPS);
        while(!editorEvents());
    }
    return 0;
}
