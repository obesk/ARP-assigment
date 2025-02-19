
#include <unistd.h>
#define PROCESS_NAME "TARGETS"

#include "blackboard.h"
#include "logging.h"
#include "target.h"

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define PERIOD 10000000

int main(int argc, char **argv) {
	// this is to prevent the other processes which can be spawned at the same
	// time to have the same seed
	srand(time(NULL) ^ getpid());
	log_message(LOG_INFO, PROCESS_NAME, "Obstacles running");

	if (argc != 3) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Incorrect number of arguments, expected: 3, received: %d",
					argc);
		exit(1);
	}

	// TODO: add error check
	int rpfd = atoi(argv[1]);
	int wpfd = atoi(argv[2]);

	while (true) {
		// messageWrite(&get_drone_position, wpfd, rpfd);
		struct Obstacles obstacles;

		for (int i = 0; i < MAX_TARGETS; ++i) {
			// TODO: should probabily check that the targets do not spawn in the
			// same coordinates as the drone
			obstacles.obstacles[i] = vec2D_random(0, GEOFENCE);
			log_message(LOG_INFO, PROCESS_NAME,
						"generated obstacle with x: %f, y %f",
						obstacles.obstacles[i].x, obstacles.obstacles[i].y);
		}

		obstacles.n = MAX_OBSTACLES;
		blackboard_set(SECTOR_OBSTACLES,
					   &(union Payload){.obstacles = obstacles}, wpfd, rpfd);
		usleep(PERIOD);
	}
	close(rpfd);
	close(wpfd);

	return 0;
}
