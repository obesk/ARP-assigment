#define PROCESS_NAME "MAP"

#include "blackboard.h"
#include "processes.h"
#include "time_management.h"
#include "vec2d.h"
#include "watchdog.h"

#include <ncurses.h>

#define PERIOD process_periods[PROCESS_MAP]
#define US_TO_S 0.000001

struct Vec2Dint {
	int x;
	int y;
};

void init_screen(void);
struct Vec2Dint convert_coordinates(const WINDOW *win, struct Vec2D coord);

int main(int argc, char **argv) {

	log_message(LOG_INFO, PROCESS_NAME, "Map running");

	if (argc < 4) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Incorrect number of arguments, expected: 4, received: %d",
					argc);

		exit(1);
	}

	init_screen();

	// TODO: add error check
	int rpfd = atoi(argv[1]);
	int wpfd = atoi(argv[2]);
	const pid_t watchdog_pid = atoi(argv[3]);

	WINDOW *border = newwin(0, 0, 0, 0);
	struct timespec start_exec_ts, end_exec_ts;
	while (1) {
		clock_gettime(CLOCK_REALTIME, &start_exec_ts);
		// TODO: should probably be done only on resize

		// log_message(LOG_INFO, PROCESS_NAME,
		// 			"win_width: %d, win_height: %d, w_m_to_char: %f, "
		// 			"h_m_to_char: %f",
		// 			win_width, win_height, w_m_to_char, h_m_to_char);

		const struct Message drone_answer =
			blackboard_get(SECTOR_DRONE_POSITION, wpfd, rpfd);

		const struct Vec2D drone_position =
			message_ok(&drone_answer) ? drone_answer.payload.drone_position
									  : (struct Vec2D){0};

		const struct Vec2Dint char_drone_position =
			convert_coordinates(border, drone_position);

		log_message(
			LOG_INFO, PROCESS_NAME,
			"drone x: %f, drone x char: %d, drone y : % f, drone y char : % d ",
			drone_position.x, char_drone_position.x, drone_position.y,
			char_drone_position.y);

		werase(border);
		box(border, 0, 0);

		mvwprintw(border, char_drone_position.y, char_drone_position.x, "%c",
				  '+');

		const struct Message targets_answer =
			blackboard_get(SECTOR_TARGETS, wpfd, rpfd);
		const struct Targets targets = message_ok(&targets_answer)
										   ? targets_answer.payload.targets
										   : (struct Targets){0};

		const struct Message obstacles_answer =
			blackboard_get(SECTOR_OBSTACLES, wpfd, rpfd);
		const struct Obstacles obstacles =
			message_ok(&obstacles_answer) ? obstacles_answer.payload.obstacles
										  : (struct Obstacles){0};

		for (int i = 0; i < targets.n; ++i) {
			const struct Vec2Dint t =
				convert_coordinates(border, targets.targets[i]);
			log_message(
				LOG_INFO, PROCESS_NAME,
				"target x: %f, target x char: %d, target y : % f, target y "
				"char : % d ",
				targets.targets[i].x, t.x, targets.targets[i].y, t.y);
			mvwprintw(border, t.y, t.x, "%d", targets.n - i);
		}

		for (int i = 0; i < obstacles.n; ++i) {
			const struct Vec2Dint t =
				convert_coordinates(border, obstacles.obstacles[i]);
			mvwprintw(border, t.y, t.x, "%c", 'o');
		}

		wrefresh(border);
		refresh();
		watchdog_send_hearthbeat(watchdog_pid, PROCESS_MAP);

		clock_gettime(CLOCK_REALTIME, &end_exec_ts);
		wait_for_next_period(PERIOD, start_exec_ts, end_exec_ts);
	}

	close(rpfd);
	close(wpfd);
	endwin();
	return 0;
}

struct Vec2Dint convert_coordinates(const WINDOW *win, struct Vec2D coord) {
	int win_width, win_height;
	getmaxyx(win, win_height, win_width);

	const float w_m_to_char = (float)(win_width - 2) / GEOFENCE;
	const float h_m_to_char = (float)(win_height - 2) / GEOFENCE;

	return (struct Vec2Dint){
		.x = (int)(coord.x * w_m_to_char) + 1,
		.y = (int)(coord.y * h_m_to_char) + 1,
	};
}

void init_screen(void) {
	initscr(); // Start curses mode
	printw("Map window");
	noecho();	 // Disable echo of input characters
	curs_set(0); // Hide cursor

	if (has_colors()) {
		start_color();
		init_pair(1, COLOR_BLACK, COLOR_WHITE); // Define COLOR_PAIR(1)
	}
	curs_set(0); // Hide cursor
}
