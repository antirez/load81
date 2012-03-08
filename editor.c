#include <time.h>
#include "load81.h"
#include "drawing.h"
#include "editor.h"

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

/* ============================= Editor drawing ============================= */

void editorDrawCursor(void) {
    int x = E.cx*FONT_KERNING;
    int y = l81.height-((E.cy+1)*FONT_HEIGHT);
    int charmargin = (FONT_WIDTH-FONT_KERNING)/2;

    x += E.margin_left;
    y -= E.margin_top;
    if (!(E.cblink & 0x80)) drawBox(l81.fb,x+charmargin,y,
                                x+charmargin+FONT_KERNING-1,y+FONT_HEIGHT-1,
                                165,165,255,.5);
    E.cblink += 4;
}

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

    for (y = 0; y < E.screenrows; y++) {
        int filerow = E.rowoff+y;
        if (filerow >= E.numrows) break;
        r = &E.row[filerow];
        for (x = 0; x < E.screencols; x++) {
            int idx = x+E.coloff;
            int charx,chary;
            int tr,tg,tb;
            int line_type = editorLineType(r,filerow);

            if (idx >= r->size) break;
            charx = x*FONT_KERNING;
            chary = l81.height-((y+1)*FONT_HEIGHT);
            charx += E.margin_left;
            chary -= E.margin_top;
            switch(line_type) {
            case LINE_TYPE_ERROR: tr = 255; tg = 100, tb = 100; break;
            case LINE_TYPE_COMMENT: tr = 180, tg = 180, tb = 0; break;
            default: tr = 165; tg = 165, tb = 255; break;
            }
            bfWriteChar(l81.fb,charx,chary,r->chars[idx],tr,tg,tb,1);
        }
    }
    if (l81.luaerr) {
        char *p = strchr(l81.luaerr,':');
        p = p ? p+1 : l81.luaerr;
        bfWriteString(l81.fb,E.margin_left,10,p,strlen(p),0,0,0,1);
    }
}

void editorDrawPowerOff(int x, int y) {
    drawEllipse(l81.fb,x,y,12,12,66,66,231,1);
    drawEllipse(l81.fb,x,y,7,7,165,165,255,1);
    drawBox(l81.fb,x-4,y,x+4,y+12,165,165,255,1);
    drawBox(l81.fb,x-2,y,x+2,y+14,66,66,231,1);
}

void editorDrawSaveIcon(int x, int y) {
    drawBox(l81.fb,x-12,y-12,x+12,y+12,66,66,231,1);
    drawBox(l81.fb,x-1,y+7,x+1,y+11,165,165,255,1);
    drawEllipse(l81.fb,x,y,4,4,165,165,255,1);
}

void editorDraw() {
    drawBox(l81.fb,0,0,l81.width-1,l81.height-1,165,165,255,1);
    drawBox(l81.fb,
            E.margin_left,
            E.margin_top,
            l81.width-1-E.margin_right,
            l81.height-1-E.margin_bottom,66,66,231,1);
    editorDrawChars();
    editorDrawCursor();
    /* Show buttons */
    editorDrawPowerOff(POWEROFF_BUTTON_X,POWEROFF_BUTTON_Y);
    if (E.dirty) editorDrawSaveIcon(SAVE_BUTTON_X,SAVE_BUTTON_Y);
    /* Show info about the current file */
    bfWriteString(l81.fb,E.margin_left,l81.height-E.margin_top+4,l81.filename,
        strlen(l81.filename), 255,255,255,1);
}

/* ========================= Editor events handling  ======================== */

int editorEvents(int errorLine, char *errorMsg) {
    SDL_Event event;
    int j, ksym;
    time_t idletime;

    l81.luaerrline = errorLine;
    l81.luaerr = errorMsg;

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
    sdlShowRgb(l81.screen, l81.fb);
    return 0;
}

/* As long as a key is pressed, we incremnet a counter in order to
 * implement first pression of key and key repeating.
 *
 * This function returns if the key was just pressed or if it is repeating. */
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

/* ======================= Editor file handling ======================= */

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
