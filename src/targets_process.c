#define PROCESS_NAME "TARGETS"

#include "blackboard.h"
#include "logging.h"
#include "processes.h"
#include "target.h"
#include "time_management.h"
#include "watchdog.h"

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

	struct timespec start_exec_ts, end_exec_ts;
	while (true) {
		clock_gettime(CLOCK_REALTIME, &start_exec_ts);

		// checking the state of the targets to see if they need to be updated
		const struct Message targets_answer =
			blackboard_get(SECTOR_TARGETS, wpfd, rpfd);

		const struct Targets targets = message_ok(&targets_answer)
										   ? targets_answer.payload.targets
										   : (struct Targets){0};

		if (targets.n > 0) {
			goto sleep;
		}

		const struct Message config_answer =
			blackboard_get(SECTOR_CONFIG, wpfd, rpfd);
		struct Config config = message_ok(&config_answer)
								   ? config_answer.payload.config
								   : (struct Config){0};

		struct Targets new_targets = { .n = config.n_targets };

		new_targets.n = MAX_TARGETS;
		for (int i = 0; i < new_targets.n; ++i) {
			// TODO: should probabily check that the targets do not spawn in the
			// same coordinates as the drone
			new_targets.targets[i] = vec2D_random(0, GEOFENCE);
			log_message(LOG_INFO, PROCESS_NAME,
						"generated target with x: %f, y %f",
						new_targets.targets[i].x, new_targets.targets[i].y);
		}

		blackboard_set(SECTOR_TARGETS, &(union Payload){.targets = new_targets},
					   wpfd, rpfd);

	sleep:
		watchdog_send_hearthbeat(watchdog_pid, PROCESS_TARGETS);
		clock_gettime(CLOCK_REALTIME, &end_exec_ts);
		wait_for_next_period(PERIOD, start_exec_ts, end_exec_ts);
	}
	close(rpfd);
	close(wpfd);

	return 0;
}
