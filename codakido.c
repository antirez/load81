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
#include <SDL.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <errno.h>
#include <ctype.h>

#define NOTUSED(V) ((void) V)

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600
#define FONT_WIDTH 16
#define FONT_HEIGHT 16
#define FONT_KERNING 10

#define POWEROFF_BUTTON_X   (ck.width-18)
#define POWEROFF_BUTTON_Y   18

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
    unsigned char *p;
    int width;
    int height;
} frameBuffer;

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
} ck;

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
} E;

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
            s[0] = p[2];
            s[1] = p[1];
            s[2] = p[0];
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
    #include "bitfont.h"
}

void bfWriteChar(frameBuffer *fb, int xp, int yp, int c, int r, int g, int b, float alpha) {
    int x,y;
    unsigned char *bitmap = ck.font[c&0xff];

    if (!bitmap) bitmap = ck.font['?'];
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

/* ========================= Lua helper functions ========================== */

/* Set a Lua global to the specified number. */
void setNumber(char *name, lua_Number n) {
    lua_pushnumber(ck.L,n);
    lua_setglobal(ck.L,name);
}

/* Get a Lua global containing a number. */
lua_Number getNumber(char *name) {
    lua_Number n;

    lua_getglobal(ck.L,name);
    n = lua_tonumber(ck.L,-1);
    lua_pop(ck.L,1);
    return n;
}

/* Set a Lua global table field to the specified value.
 * If s == NULL the field is set to the specified number 'n',
 * otherwise it is set to the specified string 's'. */
void setTableField(char *name, char *field, char *s, lua_Number n) {
    lua_getglobal(ck.L,name);
    /* Create the table if needed */
    if (lua_isnil(ck.L,-1)) {
        lua_pop(ck.L,1);
        lua_newtable(ck.L);
        lua_setglobal(ck.L,name);
        lua_getglobal(ck.L,name);
    }
    /* Set the field */
    if (lua_istable(ck.L,-1)) {
        lua_pushstring(ck.L,field);
        if (s != NULL)
            lua_pushstring(ck.L,s);
        else
            lua_pushnumber(ck.L,n);
        lua_settable(ck.L,-3);
    }
    lua_pop(ck.L,1);
}

/* ============================= Lua bindings ============================== */
int fillBinding(lua_State *L) {
    ck.r = lua_tonumber(L,-4);
    ck.g = lua_tonumber(L,-3);
    ck.b = lua_tonumber(L,-2);
    ck.alpha = lua_tonumber(L,-1);
    if (ck.r < 0) ck.r = 0;
    if (ck.r > 255) ck.r = 255;
    if (ck.g < 0) ck.g = 0;
    if (ck.g > 255) ck.g = 255;
    if (ck.b < 0) ck.b = 0;
    if (ck.b > 255) ck.b = 255;
    if (ck.alpha < 0) ck.alpha = 0;
    if (ck.alpha > 1) ck.alpha = 1;
    return 0;
}

int rectBinding(lua_State *L) {
    int x,y,w,h;

    x = lua_tonumber(L,-4);
    y = lua_tonumber(L,-3);
    w = lua_tonumber(L,-2);
    h = lua_tonumber(L,-1);
    drawBox(ck.fb,x,y,x+w,y+h,ck.r,ck.g,ck.b,ck.alpha);
    return 0;
}

int ellipseBinding(lua_State *L) {
    int x,y,rx,ry;

    x = lua_tonumber(L,-4);
    y = lua_tonumber(L,-3);
    rx = lua_tonumber(L,-2);
    ry = lua_tonumber(L,-1);
    drawEllipse(ck.fb,x,y,rx,ry,ck.r,ck.g,ck.b,ck.alpha);
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
    drawTriangle(ck.fb,x1,y1,x2,y2,x3,y3,ck.r,ck.g,ck.b,ck.alpha);
    return 0;
}

int lineBinding(lua_State *L) {
    int x1,y1,x2,y2;

    x1 = lua_tonumber(L,-4);
    y1 = lua_tonumber(L,-3);
    x2 = lua_tonumber(L,-2);
    y2 = lua_tonumber(L,-1);
    drawLine(ck.fb,x1,y1,x2,y2,ck.r,ck.g,ck.b,ck.alpha);
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
    bfWriteString(ck.fb,x,y,s,len,ck.r,ck.g,ck.b,ck.alpha);
    return 0;
}

int backgroundBinding(lua_State *L) {
    int r,g,b;

    r = lua_tonumber(L,-3);
    g = lua_tonumber(L,-2);
    b = lua_tonumber(L,-1);
    drawBox(ck.fb,0,0,ck.width-1,ck.height-1,r,g,b,1);
    return 0;
}

/* ========================== Events processing ============================= */

void setup(void) {
    lua_getglobal(ck.L,"setup");
    if (!lua_isnil(ck.L,-1)) {
        if (lua_pcall(ck.L,0,0,0)) {
            printf("Setup: %s\n",lua_tostring(ck.L,-1));
            exit(1);
        }
    } else {
        lua_pop(ck.L,1);
    }
}

void draw(void) {
    lua_getglobal(ck.L,"draw");
    if (!lua_isnil(ck.L,-1)) {
        if (lua_pcall(ck.L,0,0,0)) {
            printf("Draw: %s\n",lua_tostring(ck.L,-1));
            exit(1);
        }
    } else {
        lua_pop(ck.L,1);
    }
}

/* Update the keyboard.pressed and mouse.pressed Lua table. */
void updatePressedState(char *object, char *keyname, int pressed) {
    lua_getglobal(ck.L,object);         /* $keyboard */
    lua_pushstring(ck.L,"pressed");     /* $keyboard, "pressed" */
    lua_gettable(ck.L,-2);              /* $keyboard, $pressed */
    lua_pushstring(ck.L,keyname);       /* $keyboard, $pressed, "keyname" */
    if (pressed) {
        lua_pushboolean(ck.L,1);        /* $k, $pressed, "keyname", true */
    } else {
        lua_pushnil(ck.L);              /* $k, $pressed, "keyname", nil */
    }
    lua_settable(ck.L,-3);              /* $k, $pressed */
    lua_pop(ck.L,2);
}

void keyboardEvent(SDL_KeyboardEvent *key, int down) {
    char *keyname = SDL_GetKeyName(key->keysym.sym);

    setTableField("keyboard","state",down ? "down" : "up",0);
    setTableField("keyboard","key",keyname,0);
    updatePressedState("keyboard",keyname,down);
}

void mouseMovedEvent(int x, int y, int xrel, int yrel) {
    setTableField("mouse","x",NULL,x);
    setTableField("mouse","y",NULL,ck.height-1-y);
    setTableField("mouse","xrel",NULL,xrel);
    setTableField("mouse","yrel",NULL,-yrel);
}

void mouseButtonEvent(int button, int pressed) {
    char buttonname[32];
    
    snprintf(buttonname,sizeof(buttonname),"%d",button);
    updatePressedState("mouse",buttonname,pressed);
}

void resetEvents(void) {
    setTableField("keyboard","state","none",0);
    setTableField("keyboard","key","",0);
}

int processSdlEvents(void) {
    SDL_Event event;

    resetEvents();
    if (SDL_PollEvent(&event)) {
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
        }
    }

    /* Call the setup function, only the first time. */
    if (ck.epoch == 0) setup();
    /* Call the draw function at every iteration.  */
    draw();
    ck.epoch++;
    /* Refresh the screen */
    sdlShowRgb(ck.screen,ck.fb);
    return 0;
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
}

/* Remove the row at the specified position, shifting the remainign on the
 * top. */
void editorDelRow(int at) {
    if (at >= E.numrows) return;
    memmove(E.row+at,E.row+at+1,sizeof(E.row[0])*(E.numrows-at-1));
    E.numrows--;
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
        memmove(row->chars+at+1,row->chars+at,row->size-at);
        row->size++;
    }
    row->chars[at] = c;
}

/* Append the string 's' at the end of a row */
void editorRowAppendString(erow *row, char *s) {
    int l = strlen(s);

    row->chars = realloc(row->chars,row->size+l+1);
    memcpy(row->chars+row->size,s,l);
    row->size += l;
    row->chars[row->size] = '\0';
}

void editorRowDelChar(erow *row, int at) {
    if (row->size <= at) return;
    memmove(row->chars+at,row->chars+at+1,row->size-at);
    row->size--;
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
}

/* Inserting a newline is slightly complex as we have to handle inserting a
 * newline in the middle of a line, splitting the line as needed. */
void editorInsertNewline(void) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
    erow *row = (filerow >= E.numrows) ? NULL : &E.row[filerow];

    if (!row) return;
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
}

/* ============================= Editor drawing ============================= */

void editorDrawCursor(void) {
    int x = E.cx*FONT_KERNING;
    int y = ck.height-((E.cy+1)*FONT_HEIGHT);
    int charmargin = (FONT_WIDTH-FONT_KERNING)/2;

    x += E.margin_left;
    y -= E.margin_top;
    if (!(E.cblink & 0x80)) drawBox(ck.fb,x+charmargin,y,
                                x+charmargin+FONT_KERNING-1,y+FONT_HEIGHT-1,
                                165,165,255,.5);
    E.cblink += 4;
}

void editorDrawChars(void) {
    int y,x;
    erow *r;

    for (y = 0; y < E.screenrows; y++) {
        if (E.rowoff+y >= E.numrows) break;
        r = &E.row[E.rowoff+y];
        for (x = 0; x < E.screencols; x++) {
            int idx = x+E.coloff;
            int charx,chary;

            if (idx >= r->size) break;
            charx = x*FONT_KERNING;
            chary = ck.height-((y+1)*FONT_HEIGHT);
            charx += E.margin_left;
            chary -= E.margin_top;
            bfWriteChar(ck.fb,charx,chary,r->chars[idx],165,165,255,1);
        }
    }
}

void editorDrawPowerOff(int x, int y) {
    drawEllipse(ck.fb,x,y,12,12,66,66,231,1);
    drawEllipse(ck.fb,x,y,7,7,165,165,255,1);
    drawBox(ck.fb,x-4,y,x+4,y+12,165,165,255,1);
    drawBox(ck.fb,x-2,y,x+2,y+14,66,66,231,1);
}

void editorDraw() {
    drawBox(ck.fb,0,0,ck.width-1,ck.height-1,165,165,255,1);
    drawBox(ck.fb,
            E.margin_left,
            E.margin_top,
            ck.width-1-E.margin_right,
            ck.height-1-E.margin_bottom,66,66,231,1);
    editorDrawChars();
    editorDrawCursor();
    /* Show buttons */
    editorDrawPowerOff(POWEROFF_BUTTON_X,POWEROFF_BUTTON_Y);
    /* Show info about the current file */
    bfWriteString(ck.fb,E.margin_left,ck.width-E.margin_top+2,ck.filename,
        strlen(ck.filename), 255,255,255,1);
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
    }
}

void editorMoveCursor(int key) {
    int filerow = E.rowoff+E.cy;
    int filecol = E.coloff+E.cx;
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

    if (SDL_PollEvent(&event)) {
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
            editorMouseClicked(event.motion.x, ck.height-1-event.motion.y,
                               event.button.button);
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
    sdlShowRgb(ck.screen,ck.fb);
    return 0;
}

/* =========================== Initialization ============================== */

void initConfig(void) {
    ck.screen = NULL;
    ck.width = DEFAULT_WIDTH;
    ck.height = DEFAULT_HEIGHT;
    ck.r = 255;
    ck.g = ck.b = 0;
    ck.alpha = 1;
    ck.L = NULL;

    /* Load the bitmap font */
    bfLoadFont((char**)ck.font);
}

/* Load the specified program in the editor memory and returns 0 on success
 * or 1 on error. */
int editorOpenProgram(char *filename) {
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
    return 0;
}

/* Load the editor program into Lua. Returns 0 on success, 1 on error. */
int loadProgram(void) {
    int buflen;
    char *buf = editorRowsToString(&buflen);

    if (luaL_loadbuffer(ck.L,buf,buflen,ck.filename)) {
        fprintf(stderr, "Couldn't load file: %s\n", lua_tostring(ck.L, -1));
        free(buf);
        return 1;
    }
    free(buf);
    if (lua_pcall(ck.L,0,0,0)) {
        fprintf(stderr, "Error running script: %s\n", lua_tostring(ck.L,-1));
        return 1;
    }
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
    E.screencols = (ck.width-E.margin_left-E.margin_right) / FONT_KERNING;
    E.screenrows = (ck.height-E.margin_top-E.margin_bottom) / FONT_HEIGHT;
    memset(E.key,0,sizeof(E.key));
}

void initScreen(void) {
    ck.fb = createFrameBuffer(ck.width,ck.height);
    ck.screen = sdlInit(ck.width,ck.height,0);
}

void resetProgram(void) {
    char *initscript =
        "keyboard={}; keyboard['pressed']={};"
        "mouse={}; mouse['pressed']={};";

    ck.epoch = 0;
    if (ck.L) lua_close(ck.L);
    ck.L = lua_open();
    luaopen_base(ck.L);
    luaopen_table(ck.L);
    luaopen_string(ck.L);
    luaopen_math(ck.L);
    luaopen_debug(ck.L);
    setNumber("WIDTH",ck.width);
    setNumber("HEIGHT",ck.height);
    luaL_loadbuffer(ck.L,initscript,strlen(initscript),"initscript");
    lua_pcall(ck.L,0,0,0);

    /* Make sure that mouse parameters make sense even before the first
     * mouse event captured by SDL */
    setTableField("mouse","x",NULL,0);
    setTableField("mouse","y",NULL,0);
    setTableField("mouse","xrel",NULL,0);
    setTableField("mouse","yrel",NULL,0);

    /* Register API */
    lua_pushcfunction(ck.L,fillBinding);
    lua_setglobal(ck.L,"fill");
    lua_pushcfunction(ck.L,rectBinding);
    lua_setglobal(ck.L,"rect");
    lua_pushcfunction(ck.L,ellipseBinding);
    lua_setglobal(ck.L,"ellipse");
    lua_pushcfunction(ck.L,backgroundBinding);
    lua_setglobal(ck.L,"background");
    lua_pushcfunction(ck.L,triangleBinding);
    lua_setglobal(ck.L,"triangle");
    lua_pushcfunction(ck.L,lineBinding);
    lua_setglobal(ck.L,"line");
    lua_pushcfunction(ck.L,textBinding);
    lua_setglobal(ck.L,"text");
}

/* ================================= Main ================================== */

int main(int argc, char **argv) {
    NOTUSED(argc);
    NOTUSED(argv);

    if (argc != 2) {
        fprintf(stderr,"Usage: %s <filename>\n", argv[0]);
        exit(1);
    }

    initConfig();
    initEditor();
    initScreen();
    ck.filename = argv[1];
    editorOpenProgram(ck.filename);
    while(1) {
        resetProgram();
        loadProgram();
        while(!processSdlEvents());
        E.lastevent = time(NULL);
        while(!editorEvents());
    }
    return 0;
}
