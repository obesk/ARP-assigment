#define PROCESS_NAME "MAP"
#include "blackboard.h"
#include <ncurses.h>

#define PERIOD 10000
#define US_TO_S 0.000001

void init_screen(void);

int main(int argc, char **argv) {

	int win_width, win_height;

	log_message(LOG_INFO, PROCESS_NAME, "Map running");

	if (argc != 3) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Incorrect number of arguments, expected: 3, received: %d",
					argc);

		exit(1);
	}

	init_screen();

	// TODO: add error check
	int rpfd = atoi(argv[1]);
	int wpfd = atoi(argv[2]);

	WINDOW *border = newwin(0, 0, 0, 0);

	while (1) {
		// TODO: should probably be done only on resize
		getmaxyx(border, win_height, win_width);
		const float w_m_to_char = (float)win_width / GEOFENCE;
		const float h_m_to_char = (float)win_height / GEOFENCE;

		// log_message(LOG_INFO, PROCESS_NAME,
		// 			"win_width: %d, win_height: %d, w_m_to_char: %f, "
		// 			"h_m_to_char: %f",
		// 			win_width, win_height, w_m_to_char, h_m_to_char);
		box(border, 0, 0);

		wrefresh(border);

		const struct Message answer =
			blackboard_get(SECTOR_DRONE_POSITION, wpfd, rpfd);

		const struct Vec2D drone_position = message_ok(&answer)
												? answer.payload.drone_position
												: (struct Vec2D){0};

		const int char_drone_position_x = (int)(drone_position.x * w_m_to_char);
		const int char_drone_position_y = (int)(drone_position.y * h_m_to_char);

		log_message(
			LOG_INFO, PROCESS_NAME,
			"drone x: %f, drone x char: %d, drone y : % f, drone y char : % d ",
			drone_position.x, char_drone_position_x, drone_position.y,
			char_drone_position_y);

		werase(border);
		mvwprintw(border, char_drone_position_y, char_drone_position_x, "%c",
				  '+');

		wrefresh(border);
		// mvwprintw(border, 0, 0, "%c", '+');
		refresh();
		// TODO: add period
		usleep(PERIOD);
	}

	close(rpfd);
	close(wpfd);
	endwin();
	return 0;
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