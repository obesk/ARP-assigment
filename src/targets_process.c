#define PROCESS_NAME "TARGETS"

#include "vec2d.h"
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

int spawn_targets(int rpfd, int wpfd);

int main(int argc, char **argv) {
	log_message(LOG_INFO, "Targets running");

	if (argc < 4) {
		log_message(LOG_CRITICAL,
					"Incorrect number of arguments, expected: 4, received: %d",
					argc);
		exit(1);
	}

	int rpfd, wpfd, watchdog_pid; 
	if (!process_get_arguments(argv, &rpfd, &wpfd, &watchdog_pid)) {
		exit(1);
	}
	
	watchdog_register_term_handler();

	// this is to prevent the other processes which can be spawned at the same
	// time to have the same seed
	srand(time(NULL) ^ getpid());

	int current_target_n = spawn_targets(rpfd, wpfd);

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
		current_target_n = spawn_targets(rpfd,wpfd);

	sleep:
		watchdog_send_hearthbeat(watchdog_pid, PROCESS_TARGETS);
		clock_gettime(CLOCK_REALTIME, &end_exec_ts);
		wait_for_next_period(PERIOD, start_exec_ts, end_exec_ts);
	}
	close(rpfd);
	close(wpfd);

	return 0;
}

int spawn_targets(int rpfd, int wpfd) {
	struct Config config = blackboard_get_config(wpfd, rpfd);
	struct Targets new_targets = { .n = config.n_targets };

	struct Vec2D drone_position = blackboard_get_drone_position(wpfd, rpfd);

	log_message(LOG_INFO, "spawning %d targets:", new_targets.n);

	for (int i = 0; i < new_targets.n; ++i) {
		// making shure that the targets are not spawned in the drone position
		// or in the position of other obstacles
		bool unique_position;
		do {
			unique_position = true;
			new_targets.targets[i] = vec2D_random(0, GEOFENCE);
			for (int j = 0; j < i; ++j) {
				if (vec2D_equals(new_targets.targets[j], new_targets.targets[i])) {
					unique_position = false;
				}
			}
			unique_position &= !vec2D_equals(new_targets.targets[i], drone_position);
		} while(!unique_position);

		log_message(LOG_INFO,
					"generated target with x: %f, y %f",
					new_targets.targets[i].x, new_targets.targets[i].y);
	}

	blackboard_set(SECTOR_TARGETS, &(union Payload){.targets = new_targets},
				   wpfd, rpfd);
	return new_targets.n;
}
