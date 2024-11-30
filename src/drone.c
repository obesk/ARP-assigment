#define PROCESS_NAME "DRONE"

#include "blackboard.h"
#include "logging.h"

#include <unistd.h>

int main(int argc, char **argv) {
	log_message(LOG_INFO, PROCESS_NAME, "Drone running");
	Message request = {
		.type = RESPONSE,
		.sector = DRONE_FORCES,
	};

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

	sleep(1);
	log_message(LOG_DEBUG, PROCESS_NAME, "writing message to blackboard");
	writeMsg(&request, wpfd);
	sleep(1);
	log_message(LOG_DEBUG, PROCESS_NAME,
				"writing another message to blackboard");
	writeMsg(&request, wpfd);
	close(wpfd);
	close(rpfd);
}
