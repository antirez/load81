#ifndef EDITOR_H
#define EDITOR_H

#include "framebuffer.h"

#define POWEROFF_BUTTON_X   (E.fb->width-18)
#define POWEROFF_BUTTON_Y   18
#define SAVE_BUTTON_X       (E.fb->width-E.margin_right-13)
#define SAVE_BUTTON_Y       (E.fb->height-16)
#define EDITOR_FPS 30

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

#endif /* EDITOR_H */
