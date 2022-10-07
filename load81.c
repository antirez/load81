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

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "framebuffer.h"
#include "editor.h"
#include "load81.h"

#define NOTUSED(V) ((void) V)

struct globalConfig l81;

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
    int line = 0;
    char *p;

    if ((p = strchr(e,':')) != NULL)
        line = atoi(p+1)-1;
    editorSetError(e,line);
    l81.luaerr = 1;
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
    SDL_setFramerate(&l81.fb->fps_mgr,l81.fps);
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
    int x, y;
    unsigned char rgb[3];

    x = lua_tonumber(L,-2);
    y = l81.fb->height - 1 - lua_tonumber(L,-1);

    if (x < 0 || x >= l81.fb->width || y < 0 || y >= l81.fb->height) {
        rgb[0] = rgb[1] = rgb[2] = 0;
    } else {
        SDL_Rect rect = {x,y,1,1};
        SDL_RenderReadPixels(l81.fb->renderer,&rect,SDL_PIXELFORMAT_BGR888,
                             rgb,3*l81.fb->width);
    }
    /* Return the pixel as three values: r, g, b. */
    lua_pushnumber(L,rgb[0]);
    lua_pushnumber(L,rgb[1]);
    lua_pushnumber(L,rgb[2]);
    return 3;
}

int spriteBinding(lua_State *L) {
    const char *filename;
    int x, y, angle, antialiasing;
    void *sprite;

    filename = lua_tostring(L, 1);
    x = lua_tonumber(L, 2);
    y = lua_tonumber(L, 3);
    angle = luaL_optnumber(L,4,0);
    antialiasing = lua_toboolean(L,5);
    sprite = spriteLoad(L,filename);
    spriteBlit(l81.fb, sprite, x, y, angle, antialiasing);
    return 1;
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
    char *keyname = (char*)SDL_GetKeyName(key->keysym.sym);
    size_t keylen = strlen(keyname);

    /* SDL2 key names are no longer lowercase by default, so we need
     * to convert them. */
    char buf[32];
    if (keylen >= sizeof(buf)) keylen = sizeof(buf)-1;
    for (size_t j = 0; j < keylen; j++) buf[j] = tolower(keyname[j]);
    buf[keylen] = 0;

    setTableFieldString("keyboard","state",down ? "down" : "up");
    setTableFieldString("keyboard","key",buf);
    updatePressedState("keyboard",buf,down);
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
        if (SDL_PeepEvents(&event,1,SDL_PEEKEVENT,SDL_FIRSTEVENT,
                                    SDL_LASTEVENT))
        {
            if (event.type == SDL_KEYUP ||
                event.type == SDL_MOUSEBUTTONUP)
                break; /* Go to lua before processing more. */
        }
    }

    /* Call the setup function, only the first time. */
    if (l81.epoch == 0) {
        setup();
        if (l81.luaerr) return l81.luaerr;
    }
    /* Call the draw function at every iteration.  */
    draw();
    l81.epoch++;
    /* Refresh the screen */
    if (l81.opt_show_fps) showFPS();
    presentFrameBuffer(l81.fb);
    /* Stop execution on error */
    return l81.luaerr;
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
    l81.luaerr = 0;
    l81.opt_show_fps = 0;
    l81.opt_full_screen = 0;
    l81.filename = NULL;
    srand(mstime());
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
    l81.luaerr = 0;
    editorClearError();
    return 0;
}

void initScreen(void) {
    l81.fb = createFrameBuffer(l81.width,l81.height,
                               l81.opt_full_screen);
}

void resetProgram(void) {
    char *initscript =
        "keyboard={}; keyboard['pressed']={};"
        "mouse={}; mouse['pressed']={};"
        "sprites={}";

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
    lua_pushcfunction(l81.L,spriteBinding);
    lua_setglobal(l81.L,"sprite");

    initSpriteEngine(l81.L);

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
    initScreen();
    initEditor(l81.fb,30,30,30,30);
    editorOpen(l81.filename);
    while(1) {
        resetProgram();
        loadProgram();
        if (!l81.luaerr) {
            SDL_setFramerate(&l81.fb->fps_mgr,l81.fps);
            l81.start_ms = mstime();
            while(!processSdlEvents());
            if (editorFileWasModified()) editorSave(l81.filename);
        }
        editorRun();
    }
    return 0;
}
