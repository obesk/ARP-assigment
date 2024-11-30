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

	Message request = {
		.type = RESPONSE,
		.sector = DRONE_POSITION,
	};

	log_message(LOG_DEBUG, PROCESS_NAME, "read: %d, write: %d", rpfd, wpfd);

	sleep(1);
	log_message(LOG_DEBUG, PROCESS_NAME, "writing message to blackboard");
	Message response = messageGet(&request, wpfd, rpfd);

	(void)response;

	close(wpfd);
	close(rpfd);

	return 0;
}
