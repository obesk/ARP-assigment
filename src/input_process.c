#include "vec2d.h"
#define PROCESS_NAME "INPUT"

#include "blackboard.h"
#include "keys.h"
#include "logging.h"
#include "processes.h"
#include "time_management.h"
#include "watchdog.h"

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N_FORCE 1
// defined to avoid calling cos / sin multiple times
#define COS_SIN_45 0.7071067

#define BTN_COL_DIST 11
#define BTN_ROW_DIST 5
#define BTN_COLS_N 3
#define BTN_ROWS_N (DIR_N / BTN_COLS_N)

// it's better to have odd numbers to be able to print the button on the center
#define BTN_WIDTH 5
#define BTN_HEIGHT 5

// for the window size we need a row/col less plus the button height/width
#define BTN_WIN_HEIGHT (BTN_ROW_DIST * (BTN_COLS_N - 1) + BTN_WIDTH + 2)
#define BTN_WIN_WITDTH (BTN_COL_DIST * (BTN_ROWS_N - 1) + BTN_HEIGHT + 2)

#define DATA_WIN_HEIGHT BTN_WIN_HEIGHT
#define DATA_WIN_WIDTH 50

#define PERIOD process_periods[PROCESS_INPUT]

struct ButtonWindow {
	WINDOW *win;
	WINDOW *btn_wins[DIR_N];
};

void init_screen(void);
void initialize_btn_windows(struct ButtonWindow *btn_win);
void initialize_data_window(WINDOW **win);
void draw_buttons(struct ButtonWindow *btn_win, bool btn_highlights[DIR_N]);
void draw_data(WINDOW *data_win);

int main(int argc, char **argv) {
	log_message(LOG_INFO, "Input running");

	if (argc < 4) {
		log_message(LOG_CRITICAL,
					"Incorrect number of arguments, expected: 3, received: %d",
					argc);
		exit(1);
	}

	int rpfd, wpfd, watchdog_pid; 
	if (!process_get_arguments(argv, &rpfd, &wpfd, &watchdog_pid)) {
		exit(1);
	}
	keys_direction_init();

	// this array translates the input direction to forces applied to the drone
	// stop is not here on purpose
	const struct Vec2D direction_forces[DIR_N] = {
		[DIR_UP] = {.y = -N_FORCE},
		[DIR_UP_LEFT] =
			{
				.x = -N_FORCE * COS_SIN_45,
				.y = -N_FORCE * COS_SIN_45,
			},
		[DIR_LEFT] = {.x = -N_FORCE},
		[DIR_DOWN_LEFT] =
			{
				.x = -N_FORCE * COS_SIN_45,
				.y = N_FORCE * COS_SIN_45,
			},
		[DIR_DOWN] = {.y = N_FORCE},
		[DIR_DOWN_RIGHT] =
			{
				.x = N_FORCE * COS_SIN_45,
				.y = N_FORCE * COS_SIN_45,
			},
		[DIR_RIGHT] = {.x = N_FORCE},
		[DIR_UP_RIGHT] =
			{
				.x = N_FORCE * COS_SIN_45,
				.y = -N_FORCE * COS_SIN_45,
			},
	};

	bool btn_highlights[DIR_N];

	struct ButtonWindow btn_win;
	WINDOW *data_win;

	init_screen();
	initialize_btn_windows(&btn_win);
	initialize_data_window(&data_win);

	char user_input;

	struct timespec start_exec_ts, end_exec_ts;
	while (1) {
		clock_gettime(CLOCK_REALTIME, &start_exec_ts);

		// input acquisition and management
		user_input = getch();
		memset(btn_highlights, 0, sizeof(bool) * DIR_N);

		if (user_input == 'p') {
			log_message(LOG_WARN,
						"used asked to exit with input %c, exiting ....",
						user_input);
			break;
		}

		// these two conditions should be redundant, better safe thand sorry
		if (user_input == ERR || user_input < 0) {
			goto draw;
		}

		const enum Direction d = DIRECTION_KEYS[(int)user_input];

		// checking user inputted a key corresponding with a valid direction
		if ((int)d < 0) {
			log_message(LOG_WARN,
						"Invalid direction inputted, key: %c, direction %d",
						user_input, d);
			goto draw;
		}

		btn_highlights[d] = true;

		const struct Vec2D applied_force = direction_forces[d];

		log_message(LOG_INFO,
					"user inputted: %c, corresponding direction: %d",
					user_input, d);

		const struct Vec2D curr_force = blackboard_get_drone_force(wpfd, rpfd);
		
		const struct Vec2D new_force =
			d != DIR_STOP ? vec2D_sum(curr_force, applied_force)
						  : (struct Vec2D){0};

		const union Payload payload = {.drone_force = new_force};

		blackboard_set(SECTOR_DRONE_FORCE, &payload, wpfd, rpfd);

	draw:
		;
		// drawing
		const struct Vec2D curr_drone_force = 
			blackboard_get_drone_actual_force(wpfd, rpfd);
		box(data_win, 0, 0);
		mvwprintw(data_win, 1, 1, "FORCE X: %lf", curr_drone_force.x);
		mvwprintw(data_win, 2, 1, "FORCE Y: %lf", curr_drone_force.y);
		wrefresh(data_win);
		draw_buttons(&btn_win, btn_highlights);
		refresh();
		watchdog_send_hearthbeat(watchdog_pid, PROCESS_INPUT);
		clock_gettime(CLOCK_REALTIME, &end_exec_ts);
		wait_for_next_period(PERIOD, start_exec_ts, end_exec_ts);
	}

	close(rpfd);
	close(wpfd);
	endwin();
	return 0;
}

void init_screen(void) {
	initscr(); // Start curses mode
	printw("Input window");
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


void initialize_btn_windows(struct ButtonWindow *btn_win) {
	btn_win->win = newwin(BTN_WIN_HEIGHT, BTN_WIN_WITDTH, 1, 1);
	if (!btn_win->win) {
		log_message(LOG_CRITICAL, "error while creating the main button window");
		exit(1);
	}
	for (int i = 0; i < DIR_N; i++) {
		// TODO: is this +1 ok ?
		const int row = 2 + (i / 3) * BTN_ROW_DIST;
		const int col = 2 + (i % 3) * BTN_COL_DIST;
		btn_win->btn_wins[i] = subwin(btn_win->win, BTN_HEIGHT, BTN_WIDTH, row, col);
		log_message(LOG_INFO, "drawing button at x:%d y:%d", col, row);
		if (!btn_win->btn_wins[i]) {
			log_message(LOG_CRITICAL,
						"subwin() failed for index %d", i);
			endwin();
			// exit(1);
		}
	}
}

void initialize_data_window(WINDOW **win) {
	*win = newwin(DATA_WIN_HEIGHT, DATA_WIN_WIDTH, 1, BTN_WIN_WITDTH + 2);
}

void draw_buttons(struct ButtonWindow *btn_win, bool btn_highlights[DIR_N]) {
	box(btn_win->win, 0, 0);
	for (int i = 0; i < DIR_N; ++i) {
		box(btn_win->btn_wins[i], 0, 0); // draw the button box
		if (btn_highlights[i]) {
			wattron(btn_win->btn_wins[i], COLOR_PAIR(1));
		}
		mvwprintw(btn_win->btn_wins[i], BTN_HEIGHT / 2, BTN_WIDTH / 2, "%c", KEYS[i]);
		if (btn_highlights[i]) {
			wattroff(btn_win->btn_wins[i], COLOR_PAIR(1));
		}
		wrefresh(btn_win->btn_wins[i]);
	}
	wrefresh(btn_win->win);
}

