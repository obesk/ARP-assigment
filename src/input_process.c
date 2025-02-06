#define PROCESS_NAME "INPUT"

#include "logging.h"

#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>

void init_screen(void);

int main(int argc, char **argv) {

	log_message(LOG_INFO, PROCESS_NAME, "Drone running");

	if (argc != 3) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Incorrect number of arguments, expected: 3, received: %d",
					argc);
		exit(1);
	}

	// TODO: add error check
	int rpfd = atoi(argv[1]);
	int wpfd = atoi(argv[2]);

	int ch;
	int highlight = -1;

	init_screen();

	WINDOW *win = newwin(20, 20, 2, 2);
	box(win, 0, 0);

	/* while (1) { */
	/* 	ch = getch(); */
	/* 	if (ch == ERR) { */
	/* 		// No key was pressed, turn off highlight */
	/* 		highlight = -1; */
	/* 	} else { */
	/* 		switch (ch) { */
	/* 		case KEY_UP: */
	/* 		case KEY_DOWN: */
	/* 		case KEY_LEFT: */
	/* 		case KEY_RIGHT: */
	/* 			highlight = ch; // Highlight the arrow key pressed */
	/* 			break; */
	/* 		case 'q': */
	/* 			endwin(); // Exit ncurses mode */
	/* 			return 0; */
	/* 		default: */
	/* 			break; */
	/* 		} */
	/* 	} */
	/* 	refresh(); */
	/* 	usleep(50000); // Slight delay to avoid high CPU usage (50ms) */
	/* } */
	/*  */

	while (1) {
		wgetch(win);
		wrefresh(win);
		usleep(500000);
	}

	endwin(); // Clean up and close ncurses
	return 0;
}

void init_screen(void) {
	initscr();			  // Start curses mode
	cbreak();			  // Disable line buffering
	keypad(stdscr, TRUE); // Enable special keys
	noecho();			  // Disable echo of input characters
	curs_set(0);		  // Hide cursor
}
