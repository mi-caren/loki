#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "editor.h"
#include "utils.h"
#include "terminal.h"

extern struct Editor editor;
extern struct Terminal terminal;


/* MESSAGE BAR */

void messageBarDraw(struct DynamicBuffer *dbuf) {
    char buf[32];
    // move cursor to beginning message bar
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editor.view_rows + 2, 0);
    dbuf_append(dbuf, buf, strlen(buf));
    dbuf_append(dbuf, CLEAR_LINE_CURSOR_TO_END_SEQ, CLEAR_LINE_CURSOR_TO_END_SEQ_SIZE);
    int msglen = strlen(editor.message_bar_buf);
    if (msglen > terminal.screencols) msglen = terminal.screencols;
    if (msglen && time(NULL) - editor.message_bar_time < 5)
        dbuf_append(dbuf, editor.message_bar_buf, msglen);
}

void messageBarSet(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(editor.message_bar_buf, sizeof(editor.message_bar_buf), fmt, ap);
    va_end(ap);
    editor.message_bar_time = time(NULL);
}


/* INFO BAR */


void infoBarDraw(struct DynamicBuffer *dbuf) {
    char buf[32];
    // move cursor to beginning status bar
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editor.view_rows + 1, 0);
    dbuf_append(dbuf, buf, strlen(buf));

    dbuf_append(dbuf, INVERTED_COLOR_SEQ, INVERTED_COLOR_SEQ_SIZE);

    char status[terminal.screencols];
    int len = snprintf(
        status,
        terminal.screencols / 4 * 3,
        "%s %s",
        editor.filename ? editor.filename : "[New Buffer]",
        editor.dirty ? "(modified)" : ""
    );
    dbuf_append(dbuf, status, len);
    int len_s2 = snprintf(status, terminal.screencols / 4, "%d/%d lines ", editor.editing_point.cy + (editor.numrows > 0 ? 1 : 0), editor.numrows);


    while (len < (int)(terminal.screencols - len_s2)) {
        dbuf_append(dbuf, " ", 1);
        len++;
    }
    dbuf_append(dbuf, status, len_s2);
    dbuf_append(dbuf, NORMAL_FORMATTING_SEQ, NORMAL_FORMATTING_SEQ_SIZE);
}
