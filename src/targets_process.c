#include <unistd.h>
#define PROCESS_NAME "TARGETS"

#include "blackboard.h"
#include "logging.h"
#include "target.h"

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define SLEEP_TIME_US 100000

int main(int argc, char **argv) {
	srand(time(NULL));
	log_message(LOG_INFO, PROCESS_NAME, "Drone running");

	if (argc != 3) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Incorrect number of arguments, expected: 3, received: %d",
					argc);
		exit(1);
	}

	// TODO: add error check
	int rpfd = atoi(argv[1]);
	int wpfd = atoi(argv[2]);

	struct Message get_drone_position = {
		.type = SET,
		.sector = DRONE_POSITION,
	};

	while (true) {
		messageWrite(&get_drone_position, wpfd, rpfd);

		struct Message update_targets = {.type = SET, .sector = TARGETS};

		for (int i = 0; i < MAX_TARGETS; ++i) {
			// TODO: should probabily check that the targets do not spawn in the
			// same coordinates as the drone
			update_targets.payload.targets[i] = pointRandom(0, BOARD_SIZE);
		}

		messageSet(&update_targets, wpfd, rpfd);

		usleep(SLEEP_TIME_US);
	}
	close(rpfd);
	close(wpfd);

	return 0;
}
