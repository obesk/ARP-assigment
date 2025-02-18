#include <unistd.h>
#define PROCESS_NAME "TARGETS"

#include "blackboard.h"
#include "logging.h"
#include "target.h"

#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define SLEEP_TIME_US 100000

#define PERIOD 100000

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

	// struct Message get_drone_position = {
	// 	.type = TYPE_SET,
	// 	.sector = DRONE_POSITION,
	// };

	while (true) {
		// messageWrite(&get_drone_position, wpfd, rpfd);

		union Payload payload;

		for (int i = 0; i < MAX_TARGETS; ++i) {
			// TODO: should probabily check that the targets do not spawn in the
			// same coordinates as the drone
			payload.targets.targets[i] = vec2D_random(0, GEOFENCE);
		}

		blackboard_set(SECTOR_TARGETS, &payload, wpfd, rpfd);

		usleep(PERIOD);
	}
	close(rpfd);
	close(wpfd);

	return 0;
}
