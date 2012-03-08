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

#include "load81.h"
#include "drawing.h"
#include "editor.h"

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

/* Set a Lua global table field to the specified value.
 * If s == NULL the field is set to the specified number 'n',
 * otherwise it is set to the specified string 's'. */
void setTableField(char *name, char *field, char *s, lua_Number n) {
    lua_getglobal(l81.L,name);
    /* Create the table if needed */
    if (lua_isnil(l81.L,-1)) {
        lua_pop(l81.L,1);
        lua_newtable(l81.L);
        lua_setglobal(l81.L,name);
        lua_getglobal(l81.L,name);
    }
    /* Set the field */
    if (lua_istable(l81.L,-1)) {
        lua_pushstring(l81.L,field);
        if (s != NULL)
            lua_pushstring(l81.L,s);
        else
            lua_pushnumber(l81.L,n);
        lua_settable(l81.L,-3);
    }
    lua_pop(l81.L,1);
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
    l81.alpha = lua_tonumber(L,-1);
    if (l81.r < 0) l81.r = 0;
    if (l81.r > 255) l81.r = 255;
    if (l81.g < 0) l81.g = 0;
    if (l81.g > 255) l81.g = 255;
    if (l81.b < 0) l81.b = 0;
    if (l81.b > 255) l81.b = 255;
    if (l81.alpha < 0) l81.alpha = 0;
    if (l81.alpha > 1) l81.alpha = 1;
    return 0;
}

int rectBinding(lua_State *L) {
    int x,y,w,h;

    x = lua_tonumber(L,-4);
    y = lua_tonumber(L,-3);
    w = lua_tonumber(L,-2);
    h = lua_tonumber(L,-1);
    drawBox(l81.fb,x,y,x+w,y+h,l81.r,l81.g,l81.b,l81.alpha);
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

int backgroundBinding(lua_State *L) {
    int r,g,b;

    r = lua_tonumber(L,-3);
    g = lua_tonumber(L,-2);
    b = lua_tonumber(L,-1);
    drawBox(l81.fb,0,0,l81.width-1,l81.height-1,r,g,b,1);
    return 0;
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

    setTableField("keyboard","state",down ? "down" : "up",0);
    setTableField("keyboard","key",keyname,0);
    updatePressedState("keyboard",keyname,down);
}

void mouseMovedEvent(int x, int y, int xrel, int yrel) {
    setTableField("mouse","x",NULL,x);
    setTableField("mouse","y",NULL,l81.height-1-y);
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
        case SDL_QUIT:
            exit(0);
            break;
        }
    }

    /* Call the setup function, only the first time. */
    if (l81.epoch == 0) setup();
    /* Call the draw function at every iteration.  */
    draw();
    l81.epoch++;
    /* Refresh the screen */
    sdlShowRgb(l81.screen,l81.fb);
    /* Stop execution on error */
    return l81.luaerr != NULL;
}

/* =========================== Initialization ============================== */

void initConfig(void) {
    l81.screen = NULL;
    l81.width = DEFAULT_WIDTH;
    l81.height = DEFAULT_HEIGHT;
    l81.r = 255;
    l81.g = l81.b = 0;
    l81.alpha = 1;
    l81.L = NULL;
    l81.luaerr = NULL;
    l81.luaerrline = 0;

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

void initScreen(void) {
    l81.fb = createFrameBuffer(l81.width,l81.height);
    l81.screen = sdlInit(l81.width,l81.height,0);
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
    setTableField("mouse","x",NULL,0);
    setTableField("mouse","y",NULL,0);
    setTableField("mouse","xrel",NULL,0);
    setTableField("mouse","yrel",NULL,0);

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

    /* Start with a black screen */
    drawBox(l81.fb,0,0,l81.width-1,l81.height-1,0,0,0,1);
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
    initScreen();
    l81.filename = argv[1];
    initEditor();
    editorOpen(l81.filename);
    while(1) {
        resetProgram();
        loadProgram();
        if (l81.luaerr == NULL) {
            while(!processSdlEvents());
            if (E.dirty && editorSave(l81.filename) == 0) E.dirty = 0;
        }
        E.lastevent = time(NULL);
        while(!editorEvents(l81.luaerrline, l81.luaerr));
    }
    return 0;
}
