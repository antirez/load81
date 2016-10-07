#ifndef EDITOR_H
#define EDITOR_H

#include "framebuffer.h"

#define POWEROFF_BUTTON_X   (E.fb->width-18)
#define POWEROFF_BUTTON_Y   18
#define SAVE_BUTTON_X       (E.fb->width-E.margin_right-13)
#define SAVE_BUTTON_Y       (E.fb->height-16)
#define EDITOR_FPS 30

/* Syntax highlight types */
#define HL_NORMAL 0
#define HL_ERROR 1
#define HL_COMMENT 2
#define HL_KEYWORD 3
#define HL_STRING 4
#define HL_NUMBER 5
#define HL_FUNCDEF 6
#define HL_LIB 7

#define HL_NORMAL_COLOR {165,165,255}
#define HL_ERROR_COLOR {255,0,0}
#define HL_COMMENT_COLOR {180,180,0}
#define HL_KEYWORD_COLOR {50,255,50}
#define HL_STRING_COLOR {0,255,255}
#define HL_NUMBER_COLOR {225,100,100}
#define HL_FUNCDEF_COLOR {255,255,255}
#define HL_LIB_COLOR {255,0,255}

/* Key Held Modifier Bit Masks */
#define CTRL_MASK (1<<0)
#define SHIFT_MASK (1<<1)
#define ALT_MASK (1<<2)
#define META_MASK (1<<3)

typedef struct erow {
    int size;           /* Size of the row, excluding the null term. */
    char *chars;        /* Row content. */
    unsigned char *hl;  /* Syntax highlight type for each character. */
} erow;

typedef struct keyState {
    char translation;
    int counter;
} keyState;

typedef struct hlcolor {
    int r,g,b;
} hlcolor;

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
    unsigned int modifiers;  /* Key modifiers held. CTRL & SHIFT & ALT & META  */
    int dirty;      /* File modified but not saved. */
    char *filename; /* Currently open filename */
    frameBuffer *fb;    /* Framebuffer */
    char *err;          /* Error string to display, or NULL if no error. */
    int errline;        /* Error line to highlight if err != NULL. */
};

/* ================================ Prototypes ============================== */

/* editor.c */
void initEditor(frameBuffer *fb, int mt, int mb, int ml, int mr);
char *editorRowsToString(int *buflen);
int editorOpen(char *filename);
int editorSave(char *filename);
int editorEvents(void);
void editorSetError(const char *err, int line);
void editorClearError(void);
int editorFileWasModified(void);
void editorRun(void);

int getFirstNonSpace(erow *row);

#endif /* EDITOR_H */
