package main

import (
	"fmt"
	"os"
)

type editor struct {
	terminal terminal
}

func newEditor() *editor {
	terminal := terminal{}
	if err := terminal.enableRawMode(); err != nil {
		die(err, "Cannot set raw mode\r\n")
	}
	defer terminal.disableRawMode()

	if err := terminal.init(); err != nil {
		die(err, "Unable to initialize terminal\r\n")
	}

	editor := editor{
		terminal: terminal,
	}

	// editor.editing_point = (EditingPoint) 0;
	// editor.rx = 0;
	// editor.numrows = 0;
	// editor.rows = NULL;
	// editor.rowoff = 0;
	// editor.coloff = 0;
	// editor.filename = NULL;
	// editor.message_bar_buf[0] = '\0';
	// editor.message_bar_time = 0;
	// editor.dirty = false;

	// editor.searching = false;
	// editor.search_query = NULL;

	// editor.selecting = false;
	// editor.selection_start = 0;
	// editor.copy_buf = NULL;

	// editor.undoable_command_history = VEC(Command*);
	// editor.curr_cmd = NULL;

	// int height = terminal.screenrows;
	// if (height < 0) {
	//     editor.view_rows = 0;
	// } else if (height <= 2) {
	//     editor.view_rows = height;
	// }
	// editor.view_rows = height - 2;

	return &editor
}

func die(err error, errorMessage string) {
	os.Stdout.WriteString(CLEAR_SCREEN)
	os.Stdout.WriteString(MOVE_CURSOR_TO_ORIG)

	fmt.Fprintf(os.Stderr, "%s: %s", errorMessage, err)
	os.Exit(1)
}
