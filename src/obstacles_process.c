#define PROCESS_NAME "OBSTACLES"

#include "blackboard.h"
#include "logging.h"
#include "obstacle.h"
#include "processes.h"
#include "time_management.h"
#include "watchdog.h"

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define PERIOD process_periods[PROCESS_OBSTACLES]
#define OBSTACLE_UPDATE_TIME 100000000

int main(int argc, char **argv) {
	log_message(LOG_INFO, "Obstacles running");

	int rpfd, wpfd, watchdog_pid;
	if (!process_get_arguments(argv, &rpfd, &wpfd, &watchdog_pid)) {
		exit(1);
	}

	watchdog_register_term_handler();

	// this is to prevent the other processes which can be spawned at the same
	// time to have the same seed
	srand(time(NULL) ^ getpid());

	if (argc != 4) {
		log_message(LOG_CRITICAL,
					"Incorrect number of arguments, expected: 4, received: %d",
					argc);
		exit(1);
	}

	const int obstacle_update_cycles = OBSTACLE_UPDATE_TIME / PERIOD;
	int cycles = 0;

	struct timespec start_exec_ts, end_exec_ts;
	while (true) {
		clock_gettime(CLOCK_REALTIME, &start_exec_ts);
		// messageWrite(&get_drone_position, wpfd, rpfd);

		if (cycles % obstacle_update_cycles) {
			goto sleep;
		}

		struct Config config = blackboard_get_config(wpfd, rpfd);
		struct Obstacles obstacles = {.n = config.n_obstacles};

		struct Vec2D drone_position = blackboard_get_drone_position(wpfd, rpfd);
		for (int i = 0; i < obstacles.n; ++i) {
			bool unique_position;
			do {
				unique_position = true;
				obstacles.obstacles[i] = vec2D_random(0, GEOFENCE);
				for (int j = 0; j < i; ++j) {
					if (vec2D_equals(obstacles.obstacles[j],
									 obstacles.obstacles[i])) {
						unique_position = false;
					}
				}
				unique_position &=
					!vec2D_equals(obstacles.obstacles[i], drone_position);
			} while (!unique_position);

			log_message(LOG_INFO, "generated obstacle with x: %f, y %f",
						obstacles.obstacles[i].x, obstacles.obstacles[i].y);
		}

		blackboard_set(SECTOR_OBSTACLES,
					   &(union Payload){.obstacles = obstacles}, wpfd, rpfd);

	sleep:
		watchdog_send_hearthbeat(watchdog_pid, PROCESS_OBSTACLES);
		++cycles;

		clock_gettime(CLOCK_REALTIME, &end_exec_ts);
		wait_for_next_period(PERIOD, start_exec_ts, end_exec_ts);
	}

	close(rpfd);
	close(wpfd);

	return 0;
}
