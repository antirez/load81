#ifndef __LOAD81_EDITOR__
#define __LOAD81_EDITOR__

#include <time.h>
#include "load81.h"
#include "drawing.h"

#define LINE_TYPE_NORMAL 0
#define LINE_TYPE_COMMENT 1
#define LINE_TYPE_ERROR 2

/* As long as a key is pressed, we incremnet a counter in order to
 * implement first pression of key and key repeating.
 *
 * This function returns if the key was just pressed or if it is repeating. */
#define KEY_REPEAT_PERIOD 4
#define KEY_REPEAT_PERIOD_FAST 1
#define KEY_REPEAT_DELAY 16

#define FONT_WIDTH 16
#define FONT_HEIGHT 16

#define POWEROFF_BUTTON_X   (l81.width-18)
#define POWEROFF_BUTTON_Y   18
#define SAVE_BUTTON_X       (l81.width-E.margin_right-13)
#define SAVE_BUTTON_Y       (l81.height-16)

#define KEY_MAX 512 /* Latest key is excluded */

/* ============================== Portable sleep ============================ */

#ifdef WIN32
#include <windows.h>
#define sleep_milliseconds(x) Sleep(x)
#else
#include <unistd.h>
#define sleep_milliseconds(x) usleep((x)*1000)
#endif

typedef struct erow {
    int size;
    char *chars;
} erow;

typedef struct keyState {
    char translation;
    int counter;
} keyState;

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

void initEditor(void);
void editorDrawCursor(void);
int editorLineType(erow *row, int filerow);
void editorDrawChars(void);
void editorDrawPowerOff(int x, int y);
void editorDrawSaveIcon(int x, int y);
void editorDraw();
int editorEvents(int errorLine, char *errorMsg);
int pressed_or_repeated(int counter);
void editorMouseClicked(int x, int y, int button);
void editorMoveCursor(int key);
void editorInsertRow(int at, char *s);
void editorDelRow(int at);
char *editorRowsToString(int *buflen);
void editorRowInsertChar(erow *row, int at, int c);
void editorRowAppendString(erow *row, char *s);
void editorRowDelChar(erow *row, int at);
void editorInsertChar(int c);
void editorInsertNewline(void);
void editorDelChar();
int editorOpen(char *filename);
int editorSave(char *filename);

#endif
