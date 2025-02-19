#include <unistd.h>
#define PROCESS_NAME "TARGETS"

#include "blackboard.h"
#include "logging.h"
#include "target.h"

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define PERIOD 1000

int main(int argc, char **argv) {
	srand(time(NULL));
	log_message(LOG_INFO, PROCESS_NAME, "Targets running");

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
		// checking the state of the targets to see if they need to be updated
		const struct Message targets_answer =
			blackboard_get(SECTOR_TARGETS, wpfd, rpfd);

		const struct Targets targets = message_ok(&targets_answer)
										   ? targets_answer.payload.targets
										   : (struct Targets){0};

		if (targets.n > 0) {
			continue;
		}

		struct Targets new_targets;
		for (int i = 0; i < MAX_TARGETS; ++i) {
			// TODO: should probabily check that the targets do not spawn in the
			// same coordinates as the drone
			new_targets.targets[i] = vec2D_random(0, GEOFENCE);
			log_message(LOG_INFO, PROCESS_NAME,
						"generated target with x: %d, y %d",
						new_targets.targets[i].x, new_targets.targets[i].y);
		}

		new_targets.n = MAX_TARGETS;
		blackboard_set(SECTOR_TARGETS, &(union Payload){.targets = new_targets},
					   wpfd, rpfd);
		usleep(PERIOD);
	}
	close(rpfd);
	close(wpfd);

	return 0;
}
