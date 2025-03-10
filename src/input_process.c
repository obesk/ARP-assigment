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

#define N_FORCE 10
// defined to avoid calling cos / sin multiple times
#define COS_SIN_45 0.7071067

#define BTN_COL_DIST 11
#define BTN_ROW_DIST 5

// it's better to have odd numbers to be able to print the button on the center
#define BTN_WIDTH 5
#define BTN_HEIGHT 5

#define PERIOD process_periods[PROCESS_INPUT]
#define US_TO_S 0.000001

void init_screen(void);
void initialize_btn_windows(WINDOW *btn_wins[DIR_N], int row, int col);
void draw_buttons(WINDOW *btn_wins[DIR_N], bool btn_highlights[DIR_N]);

int main(int argc, char **argv) {
	log_message(LOG_INFO, PROCESS_NAME, "Input running");

	if (argc < 4) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Incorrect number of arguments, expected: 3, received: %d",
					argc);
		exit(1);
	}

	// TODO: add error check
	int rpfd = atoi(argv[1]);
	int wpfd = atoi(argv[2]);
	const pid_t watchdog_pid = atoi(argv[3]);

	keys_direction_init();

	// this array translates the input direction to forces applied to the drone
	// stop is not hrere on purpose
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

	WINDOW *btn_wins[DIR_N];
	bool btn_highlights[DIR_N];

	init_screen();
	initialize_btn_windows(btn_wins, 1, 1);

	char user_input;

	struct timespec start_exec_ts, end_exec_ts;
	while (1) {
		clock_gettime(CLOCK_REALTIME, &start_exec_ts);

		// input acquisition and management
		user_input = getch();
		memset(btn_highlights, 0, sizeof(bool) * DIR_N);

		if (user_input == 'p') {
			log_message(LOG_WARN, PROCESS_NAME,
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
			log_message(LOG_WARN, PROCESS_NAME,
						"Invalid direction inputted, key: %c, direction %d",
						user_input, d);
			goto draw;
		}

		btn_highlights[d] = true;

		const struct Vec2D applied_force = direction_forces[d];

		log_message(LOG_INFO, PROCESS_NAME,
					"user inputted: %c, corresponding direction: %d",
					user_input, d);

		const struct Message answer =
			blackboard_get(SECTOR_DRONE_FORCE, wpfd, rpfd);

		// in case of error in retrieveing the data (it should not happen)
		// 0, 0 force is assumed
		const struct Vec2D curr_force = message_ok(&answer)
											? answer.payload.drone_force
											: (struct Vec2D){0};

		const struct Vec2D new_force =
			d != DIR_STOP ? vec2D_sum(curr_force, applied_force)
						  : (struct Vec2D){0};

		const union Payload payload = {.drone_force = new_force};

		blackboard_set(SECTOR_DRONE_FORCE, &payload, wpfd, rpfd);

	draw:
		// drawing
		draw_buttons(btn_wins, btn_highlights);
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
