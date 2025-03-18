package main

import (
	"errors"
	"fmt"
	"os"

	"golang.org/x/term"
)

const (
	CLEAR_SCREEN                = "\x1b[2J"
	MOVE_CURSOR_TO_ORIG         = "\x1b[H"
	MOVE_CURSOR_TO_BOTTOM_RIGHT = "\x1b[999C\x1b[999B"
	MOVE_CURSOR_TO_SEQ          = "\x1b[%d;%dH"
	CLEAR_LINE_CURSOR_TO_END    = "\x1b[K"
	HIDE_CURSOR                 = "\x1b[?25l"
	SHOW_CURSOR                 = "\x1b[?25h"
	REQUEST_CURSOR_POSITION     = "\x1b[6n"
	INVERTED_COLOR              = "\x1b[7m"
	NORMAL_FORMATTING           = "\x1b[m"
)

type cursorPosition struct {
	cx uint
	cy uint
}

type terminal struct {
	cursorPos  cursorPosition
	screenrows int
	screencols int
	oldState   *term.State
}

func (t *terminal) init() error {
	t.cursorPos.cx = 0
	t.cursorPos.cy = 0
	if err := getWindowSize(&t.screenrows, &t.screencols); err != nil {
		return err

	}

	return nil
}

func (t *terminal) enableRawMode() error {
	oldState, err := term.MakeRaw(int(os.Stdin.Fd()))
	if err != nil {
		return err
	}
	t.oldState = oldState

	return nil
}

func (t *terminal) disableRawMode() {
	term.Restore(int(os.Stdin.Fd()), t.oldState)
}

func getWindowSize(rows *int, cols *int) error {
	if _, err := os.Stdout.WriteString(MOVE_CURSOR_TO_BOTTOM_RIGHT); err != nil {
		return err
	}

	return getCursorPosition(rows, cols)
}

func getCursorPosition(rows *int, cols *int) error {
	if _, err := os.Stdout.WriteString(REQUEST_CURSOR_POSITION); err != nil {
		return err
	}

	buf := make([]byte, 32)
	for i := range 32 {
		if n, err := os.Stdin.Read(buf[i : i+1]); err != nil {
			return err
		} else if n != 1 {
			break
		}

		if buf[i] == 'R' {
			break
		}
	}

	if buf[0] != '\x1b' || buf[1] != '[' {
		return errors.New("Invalid cursor position response")
	}

	fmt.Sscanf(string(buf[2:]), "%d;%d", rows, cols)

	return nil
}
