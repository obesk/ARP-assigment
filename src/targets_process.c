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

	int current_target_n = 0;

	struct timespec start_exec_ts, end_exec_ts;
	while (true) {
		clock_gettime(CLOCK_REALTIME, &start_exec_ts);

		// checking the state of the targets to see if they need to be updated
		const struct Message targets_answer =
			blackboard_get(SECTOR_TARGETS, wpfd, rpfd);

		const struct Targets targets = message_ok(&targets_answer)
										   ? targets_answer.payload.targets
										   : (struct Targets){0};

		int caught_targets = current_target_n - targets.n;

		if (caught_targets) {
			const struct Message score_answer = blackboard_get(SECTOR_SCORE, wpfd, rpfd);
			const int score = message_ok(&score_answer) ? score_answer.payload.score : 0;
			const int new_score =  score + caught_targets * 100;
			blackboard_set(SECTOR_SCORE,
						   &(union Payload){.score = new_score }, wpfd,
						   rpfd);
			log_message(LOG_INFO, PROCESS_NAME, "updating the score on the"
					"blackboard, after detecting target caught, score: %d:,"
					"initial_score: %d, caught: %d", new_score, score,
					caught_targets);
			current_target_n = targets.n;
		} 

		if (targets.n > 0) {
			goto sleep;
		}
		
		// all targets caught, updating score and generating new targets

		const struct Message score_answer =
			blackboard_get(SECTOR_SCORE, wpfd, rpfd);

		const int score = message_ok(&score_answer) ? targets_answer.payload.score : 0;
		blackboard_set(SECTOR_SCORE,
					   &(union Payload){.score = score + 1000 }, wpfd,
					   rpfd);

		const struct Message config_answer =
			blackboard_get(SECTOR_CONFIG, wpfd, rpfd);
		struct Config config = message_ok(&config_answer)
								   ? config_answer.payload.config
								   : (struct Config){0};

		struct Targets new_targets = { .n = config.n_targets };

		log_message(LOG_INFO, PROCESS_NAME, "spawning %d targets:", new_targets.n);

		for (int i = 0; i < new_targets.n; ++i) {
			// TODO: should probabily check that the targets do not spawn in the
			// same coordinates as the drone
			new_targets.targets[i] = vec2D_random(0, GEOFENCE);
			log_message(LOG_INFO, PROCESS_NAME,
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
