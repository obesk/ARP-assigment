#include "target.h"
#define PROCESS_NAME "MAP"

#include "blackboard.h"
#include "processes.h"
#include "time_management.h"
#include "vec2d.h"
#include "watchdog.h"

#include <ncurses.h>

#define PERIOD process_periods[PROCESS_MAP]

enum CPair {
	CPAIR_DRONE = 1, // initializing to 1 since 0 is the default color
	CPAIR_TARGET,
	CPAIR_OBSTACLE,
};

struct Vec2Dint {
	int x;
	int y;
};

void init_screen(void);
struct Vec2Dint convert_coordinates(const WINDOW *win, struct Vec2D coord);

int main(int argc, char **argv) {
	log_message(LOG_INFO, "Map running");

	if (argc < 4) {
		log_message(LOG_CRITICAL,
					"Incorrect number of arguments, expected: 4, received: %d",
					argc);

		exit(1);
	}
	int rpfd, wpfd, watchdog_pid; 
	if (!process_get_arguments(argv, &rpfd, &wpfd, &watchdog_pid)) {
		exit(1);
	}

	watchdog_register_term_handler();

	init_screen();

	WINDOW *border = newwin(0, 0, 0, 0);
	struct timespec start_exec_ts, end_exec_ts;
	while (1) {
		clock_gettime(CLOCK_REALTIME, &start_exec_ts);

		const int score = blackboard_get_score(wpfd, rpfd);
		const struct Vec2D drone_position = blackboard_get_drone_position(wpfd,rpfd);

		const struct Vec2Dint char_drone_position =
			convert_coordinates(border, drone_position);

		log_message(LOG_INFO,
					"drone x: %lf, drone x char: %d, drone y : %lf, drone y "
					"char : %d ",
					drone_position.x, char_drone_position.x, drone_position.y,
					char_drone_position.y);

		werase(border);
		box(border, 0, 0);

		wattron(border, COLOR_PAIR(CPAIR_DRONE));
		mvwprintw(border, char_drone_position.y, char_drone_position.x, "%c",
				  '+');
		wattroff(border, COLOR_PAIR(CPAIR_DRONE));

		mvwprintw(border, 0, 0, "score: %d", score);

		const struct Obstacles obstacles = blackboard_get_obstacles(wpfd, rpfd);

		const struct Targets targets = blackboard_get_targets(wpfd, rpfd);
		log_message(LOG_INFO, "targets n: %d", targets.n);
		wattron(border, COLOR_PAIR(CPAIR_TARGET));
		for (int i = 0; i < targets.n; ++i) {
			const struct Vec2Dint t =
				convert_coordinates(border, targets.targets[i]);
			log_message(LOG_INFO,
				"target %d x: %lf, target x char: %d, target y : %lf, target y "
				"char : %d",
				i, targets.targets[i].x, t.x, targets.targets[i].y, t.y);
			mvwprintw(border, t.y, t.x, "%d", targets.n - i);
		}
		wattroff(border, COLOR_PAIR(CPAIR_TARGET));

		for (int i = 0; i < obstacles.n; ++i) {
			const struct Vec2Dint o =
				convert_coordinates(border, obstacles.obstacles[i]);
			log_message(LOG_INFO,
						"obstacle %d x: %lf, obstacle x char: %d, obstacle y : "
						"%lf, obstacle y "
						"char : %d ",
						i, obstacles.obstacles[i].x, o.x,
						obstacles.obstacles[i].y, o.y);
			wattron(border, COLOR_PAIR(CPAIR_OBSTACLE));
			mvwprintw(border, o.y, o.x, "%c", 'o');
			wattroff(border, COLOR_PAIR(CPAIR_OBSTACLE));
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
		init_pair(CPAIR_DRONE, COLOR_BLUE, COLOR_BLACK); 
		init_pair(CPAIR_TARGET, COLOR_YELLOW, COLOR_BLACK);  
		init_pair(CPAIR_OBSTACLE, COLOR_GREEN, COLOR_BLACK); 
	}
	curs_set(0); // Hide cursor
}
