#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdbool.h>

#include "editor/defs.h"
#include "terminal.h"
#include "editor/utils.h"
#include "utils/string.h"
#include "utils/utils.h"

#define PROMPT_CURSOR    "\033[5m_\033[0m"

extern struct Editor editor;

/* MESSAGE BAR */

void messageBarDraw(String* buf) {
    char seq_buf[32];
    // move cursor to beginning message bar
    snprintf(seq_buf, sizeof(seq_buf), "\x1b[%d;%dH", editor.view_rows + 2, 0);
    strAppend(buf, seq_buf);
    strAppend(buf, CLEAR_LINE_CURSOR_TO_END_SEQ);
    if (strlen(editor.message_bar_buf) && time(NULL) - editor.message_bar_time < 5)
        strAppend(buf, editor.message_bar_buf);
}

void messageBarSet(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(
        editor.message_bar_buf,
        umin(sizeof(editor.message_bar_buf), terminal.screencols),
        fmt,
        ap
    );
    va_end(ap);
    editor.message_bar_time = time(NULL);
}

/*
    Draw a prompt in the message bar and takes an input from the user,
    returning a buffer set with it;
    The space for the user input will be allocated
    by the function. Deallocation is done on by the caller.
    If some error occurs or if the operation is canceled, the buffer is freed by the function and NULL is returned.
    Printing of error informations to the message bar is done by this function.
*/
char* messageBarPrompt(char* prompt, int (*callback)(char*, int)) {
    size_t bufsize = 128;
    char* buf = malloc(bufsize);

    if (buf == NULL) {
        messageBarSet("Unable malloc prompt buf");
        return NULL;
    }

    size_t buflen = 0;
    buf[0] = '\0';

    while (1) {
        messageBarSet("%s (ESC to cancel): %s"PROMPT_CURSOR, prompt, buf);
        editorRefreshScreen();

        int c = editorReadKey();

        if (c == '\x1b') {
            messageBarSet("Canceled");
            free(buf);
            buf = NULL;
            break;
        } else if (c == BACKSPACE) {
            if (buflen != 0) buf[--buflen] = '\0';
        } else if (c == '\r') {
            if (buflen != 0) {
                messageBarSet("");
                break;
            }
        } else if (!iscntrl(c) && c < 128) { // if c is printable
            if (buflen == bufsize - 1) {
                bufsize *= 2;
                char* new = realloc(buf, bufsize);
                if (new == NULL) {
                    messageBarSet("Unable to realloc prompt buf");
                    free(buf);
                    buf = NULL;
                    break;
                }
                buf = new;
            }

            buf[buflen++] = c;
            buf[buflen] = '\0';
        }

        if (callback) {
            if (callback(buf, c) == -1) {
                free(buf);
                buf = NULL;
                break;
            }
        }
    }

    return buf;
}

bool messageBarPromptYesNo(char* prompt) {
    bool answer = false;
    while (1) {
        messageBarSet("%s [y/n] "PROMPT_CURSOR, prompt);
        editorRefreshScreen();
        write(STDOUT_FILENO, HIDE_CURSOR_SEQ, HIDE_CURSOR_SEQ_SIZE);

        int c = editorReadKey();

        if (c == 'y' || c == 'Y') {
            answer = true;
            break;
        } else if (c == 'n' || c == 'N') {
            messageBarSet("");
            break;
        }
    }

    // ensure cursor is visible before exiting.
    // if the call to messageBarPromptYesNo is the last before
    // exiting the editor and no other calls to editor_refresh_screen are performed
    // the user will end up with a terminal with no visible cursor
    write(STDOUT_FILENO, SHOW_CURSOR_SEQ, SHOW_CURSOR_SEQ_SIZE);
    return answer;
}

/* INFO BAR */


void infoBarDraw(String* buf) {
    char seq_buf[32];
    // move cursor to beginning status bar
    snprintf(seq_buf, sizeof(seq_buf), "\x1b[%d;%dH", editor.view_rows + 1, 0);
    strAppend(buf, seq_buf);

    strAppend(buf, INVERTED_COLOR_SEQ);

    char status[terminal.screencols];
    int len = snprintf(
        status,
        terminal.screencols / 4 * 3,
        "%s %s",
        editor.filename ? editor.filename : "[New Buffer]",
        editor.dirty ? "(modified)" : ""
    );
    strAppend(buf, status);
    int len_s2 = snprintf(
        status,
        terminal.screencols / 4,
        "%d/%zu lines ",
        getRow(editor.editing_point) + (vecLen(editor.rows) > 0 ? 1 : 0),
        vecLen(editor.rows)
    );


    strRepeatAppendChar(buf, ' ', terminal.screencols - len_s2 - len);
    strAppend(buf, status);
    strAppend(buf, NORMAL_FORMATTING_SEQ);
}
