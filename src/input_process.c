#define PROCESS_NAME "INPUT"

#include "blackboard.h"
#include "keys.h"
#include "logging.h"

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N_FORCE 1
// defined to avoid reusing cos / sin multiple times
#define COS_SIN_45 0.7071067

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

	keys_direction_init();

	// this array translates the input direction to  orces applied to the drone
	// stop not presetn on purpose
	const struct Vec2D direction_forces[DIR_N] = {
		[DIR_UP] = {.y = -N_FORCE},
		[DIR_UP_LEFT] =
			{
				.y = -N_FORCE * COS_SIN_45,
				.x = -N_FORCE * COS_SIN_45,
			},
		[DIR_LEFT] = {.x = -N_FORCE},
		[DIR_DOWN_LEFT] =
			{
				.y = N_FORCE * COS_SIN_45,
				.x = -N_FORCE * COS_SIN_45,
			},

		[DIR_DOWN] = {.y = N_FORCE},
		[DIR_DOWN_RIGHT] = {.y = N_FORCE * COS_SIN_45},

	};

	WINDOW *btn_wins[DIR_N];
	bool btn_highlights[DIR_N];

	init_screen();
	initialize_btn_windows(btn_wins, 1, 1);

	char user_input;

	while (1) {

		// input acquisition and management
		user_input = getch();
		memset(btn_highlights, 0, sizeof(bool) * DIR_N);

		if (user_input == 'p') {
			goto exit;
		}

		// these two conditions should be redundant, better safe thand sorry
		if (user_input == ERR || user_input < 0) {
			goto draw;
		}

		const enum Direction d = DIRECTION_KEYS[(int)user_input];

		// checking user inputted a key corresponding with a valid direction
		if ((int)d < 0) {
			goto draw;
			log_message(LOG_WARN, PROCESS_NAME,
						"Invalid direction inputted, key: %c, direction %d",
						user_input, d);
		}

		btn_highlights[d] = true;

		log_message(LOG_INFO, PROCESS_NAME,
					"user inputted: %c, corresponding direction: %d",
					user_input, d);

		const struct Message answer =
			blackboard_get(SECTOR_DRONE_FORCE, wpfd, rpfd);

		// in case of error in retrieveing the data (it should not happen)
		// 0, 0 position is assumed
		const struct Vec2D curr_force = message_ok(&answer)
											? answer.payload.drone_force
											: (struct Vec2D){0};

		const struct Vec2D applied_force = direction_forces[d];

		const struct Vec2D new_force =
			d != DIR_STOP ? Vec2D_sum(curr_force, applied_force)
						  : (struct Vec2D){0};

		const union Payload payload = {.drone_force = new_force};

		blackboard_set(SECTOR_DRONE_FORCE, &payload, wpfd, rpfd);

	draw:
		// drawing
		draw_buttons(btn_wins, btn_highlights);
		refresh();

		// TODO: here it would be better to define a period for every task
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
