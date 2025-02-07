#define PROCESS_NAME "INPUT"

#include "keys.h"
#include "logging.h"

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BTN_COL_DIST 11
#define BTN_ROW_DIST 5

// it's better to have odd numbers to be able to print the button on the center
#define BTN_WIDTH 5
#define BTN_HEIGHT 5

void init_screen(void);

void initialize_btn_windows(WINDOW *btn_wins[DIR_N], int row, int col);
void draw_buttons(WINDOW *btn_wins[DIR_N], bool btn_highlights[DIR_N]);

int main(int argc, char **argv) {
	log_message(LOG_INFO, PROCESS_NAME, "Input running");

	if (argc != 3) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Incorrect number of arguments, expected: 3, received: %d",
					argc);
		exit(1);
	}

	// TODO: add error check
	int rpfd = atoi(argv[1]);
	int wpfd = atoi(argv[2]);

	WINDOW *btn_wins[DIR_N];
	bool btn_highlights[DIR_N];

	init_screen();
	initialize_btn_windows(btn_wins, 1, 1);

	draw_buttons(btn_wins, btn_highlights);
	refresh();

	char user_input;

	while (1) {
		user_input = getch();
		memset(btn_highlights, 0, sizeof(bool) * DIR_N);

		if (user_input == 'p') {
			goto exit;
		}

		for (int i = 0; i < DIR_N; ++i) {
			if (user_input == KEYS[i]) {
				btn_highlights[i] = true;
			}
		}

		draw_buttons(btn_wins,
					 btn_highlights); // Redraw buttons in each loop iteration
		refresh();					  // Ensure stdscr updates the screen

		usleep(50000); // Slight delay to avoid high CPU usage (50ms)
	}

exit:
	endwin();
	return 0;
}

void initialize_btn_windows(WINDOW *btn_wins[DIR_N], int start_row,
							int start_col) {
	for (int i = 0; i < DIR_N; i++) {
		const int row = start_row + ((i / 3) * BTN_ROW_DIST);
		const int col = start_col + ((i % 3) * BTN_COL_DIST);
		btn_wins[i] = newwin(BTN_HEIGHT, BTN_WIDTH, row, col);
		if (!btn_wins[i]) {
			log_message(LOG_CRITICAL, PROCESS_NAME,
						"newwin() failed for index %d", i);
			endwin();
			exit(1);
		}
	}
}

void draw_buttons(WINDOW *btn_wins[DIR_N], bool btn_highlights[DIR_N]) {

	for (int i = 0; i < DIR_N; ++i) {
		box(btn_wins[i], 0, 0); // draw the button box
		if (btn_highlights[i]) {
			wattron(btn_wins[i], COLOR_PAIR(1));
		}
		mvwprintw(btn_wins[i], BTN_HEIGHT / 2, BTN_WIDTH / 2, "%c", KEYS[i]);
		if (btn_highlights[i]) {
			wattroff(btn_wins[i], COLOR_PAIR(1));
		}
		wrefresh(btn_wins[i]);
	}
}

void init_screen(void) {
	initscr(); // Start curses mode
	printw("Main window");
	cbreak();			  // Disable line buffering
	keypad(stdscr, TRUE); // Enable special keys
	noecho();			  // Disable echo of input characters
	curs_set(0);		  // Hide cursor
	nodelay(stdscr, TRUE);

	if (has_colors()) {
		start_color();
		init_pair(1, COLOR_BLACK, COLOR_WHITE); // Define COLOR_PAIR(1)
	}
	curs_set(0); // Hide cursor
}
