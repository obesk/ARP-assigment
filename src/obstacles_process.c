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
	// this is to prevent the other processes which can be spawned at the same
	// time to have the same seed
	srand(time(NULL) ^ getpid());
	log_message(LOG_INFO, PROCESS_NAME, "Obstacles running");

	if (argc != 4) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Incorrect number of arguments, expected: 4, received: %d",
					argc);
		exit(1);
	}

	// TODO: add error check
	int rpfd = atoi(argv[1]);
	int wpfd = atoi(argv[2]);
	int watchdog_pid = atoi(argv[3]);

	const int obstacle_update_cycles = OBSTACLE_UPDATE_TIME / PERIOD;
	int cycles = 0;

	struct timespec start_exec_ts, end_exec_ts;
	while (true) {
		clock_gettime(CLOCK_REALTIME, &start_exec_ts);
		// messageWrite(&get_drone_position, wpfd, rpfd);
		struct Obstacles obstacles;

		if (cycles % obstacle_update_cycles) {
			goto sleep;
		}

		for (int i = 0; i < MAX_OBSTACLES; ++i) {
			// TODO: should probabily check that the obstacles do not spawn in
			// the same coordinates as the drone
			obstacles.obstacles[i] = vec2D_random(0, GEOFENCE);
			log_message(LOG_INFO, PROCESS_NAME,
						"generated obstacle with x: %f, y %f",
						obstacles.obstacles[i].x, obstacles.obstacles[i].y);
		}

		obstacles.n = MAX_OBSTACLES;
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
