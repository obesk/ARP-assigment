#include "watchdog.h"
#define PROCESS_NAME "TARGETS"

#include "blackboard.h"
#include "logging.h"
#include "processes.h"
#include "target.h"

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define PERIOD process_periods[PROCESS_TARGETS]

int main(int argc, char **argv) {
	// this is to prevent the other processes which can be spawned at the same
	// time to have the same seed
	srand(time(NULL) ^ getpid());
	log_message(LOG_INFO, PROCESS_NAME, "Targets running");

	if (argc < 4) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Incorrect number of arguments, expected: 4, received: %d",
					argc);
		exit(1);
	}

	// TODO: add error check
	int rpfd = atoi(argv[1]);
	int wpfd = atoi(argv[2]);
	int watchdog_pid = atoi(argv[3]);

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
						"generated target with x: %f, y %f",
						new_targets.targets[i].x, new_targets.targets[i].y);
		}

		new_targets.n = MAX_TARGETS;
		blackboard_set(SECTOR_TARGETS, &(union Payload){.targets = new_targets},
					   wpfd, rpfd);

		watchdog_send_hearthbeat(watchdog_pid, PROCESS_TARGETS);
		usleep(PERIOD);
	}
	close(rpfd);
	close(wpfd);

	return 0;
}
