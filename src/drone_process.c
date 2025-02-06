#define PROCESS_NAME "DRONE"

#include "blackboard.h"
#include "logging.h"

#include <unistd.h>

int main(int argc, char **argv) {
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

	log_message(LOG_DEBUG, PROCESS_NAME, "read: %d, write: %d", rpfd, wpfd);

	Message position_set = {
		.type = SET,
		.sector = DRONE_POSITION,
		.payload.drone_position =
			{
				.x = 10,
				.y = 20,
			},
	};

	messageSet(&position_set, wpfd, rpfd);

	Message position_request = {
		.type = GET,
		.sector = DRONE_POSITION,
	};

	Message response = messageGet(&position_request, wpfd, rpfd);

	log_message(LOG_DEBUG, PROCESS_NAME,
				"message type: %d, sector %d, x: %f, y: %f", response.type,
				response.sector, response.payload.drone_position.x,
				response.payload.drone_position.y);

	close(wpfd);
	close(rpfd);

	return 0;
}
