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
	log_message(LOG_INFO, "Targets running");

	if (argc < 4) {
		log_message(LOG_CRITICAL,
					"Incorrect number of arguments, expected: 4, received: %d",
					argc);
		exit(1);
	}

	// TODO: add error check
	int rpfd = atoi(argv[1]);
	int wpfd = atoi(argv[2]);
	int watchdog_pid = atoi(argv[3]);

	int current_target_n = 0;

	struct timespec start_exec_ts, end_exec_ts;
	while (true) {
		clock_gettime(CLOCK_REALTIME, &start_exec_ts);

		// checking the state of the targets to see if they need to be updated
		const struct Targets targets = blackboard_get_targets(wpfd, rpfd);

		int caught_targets = current_target_n - targets.n;

		if (caught_targets) {
			const int score = blackboard_get_score(wpfd, rpfd);
			const int new_score =  score + caught_targets * 100;
			blackboard_set(SECTOR_SCORE,
						   &(union Payload){.score = new_score }, wpfd,
						   rpfd);
			log_message(LOG_INFO, "updating the score on the"
					"blackboard, after detecting target caught, score: %d:,"
					"initial_score: %d, caught: %d", new_score, score,
					caught_targets);
			current_target_n = targets.n;
		} 

		if (targets.n > 0) {
			goto sleep;
		}
		
		// all targets caught, updating score and generating new targets

		const int score = blackboard_get_score(wpfd, rpfd);
		blackboard_set(SECTOR_SCORE,
					   &(union Payload){.score = score + 1000 }, wpfd,
					   rpfd);

		struct Config config = blackboard_get_config(wpfd, rpfd);
		struct Targets new_targets = { .n = config.n_targets };

		log_message(LOG_INFO, "spawning %d targets:", new_targets.n);

		for (int i = 0; i < new_targets.n; ++i) {
			// TODO: should probabily check that the targets do not spawn in the
			// same coordinates as the drone
			new_targets.targets[i] = vec2D_random(0, GEOFENCE);
			log_message(LOG_INFO,
						"generated target with x: %f, y %f",
						new_targets.targets[i].x, new_targets.targets[i].y);
		}

		current_target_n = new_targets.n;
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
