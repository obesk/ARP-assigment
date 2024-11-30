#include "processes.h"
#define PROCESS_NAME "BLACKBOARD"

#include "blackboard.h"
#include "logging.h"
#include "pfds.h"
#include "stdbool.h"

int main(int argc, char **argv) {
	log_message(LOG_INFO, PROCESS_NAME, "Blackboard running");

	const int expected_argc = N_PROCESSES * 2 + 1; //+1 for the program name

	if (argc != expected_argc) {
		log_message(
			LOG_CRITICAL, PROCESS_NAME,
			"Erroneous number of arguments passed, expected: %d, got: %d",
			expected_argc, argc);
	}

	PFDs *pfds = argsToPFDs(&argv[1]);
	const int max_fd = getMaxFd(pfds);

	struct timeval tv;

	// Setting select timeout
	tv.tv_sec = 0;
	tv.tv_usec = 1000;

	while (true) {
		fd_set to_read;
		FD_ZERO(&to_read);

		for (int i = 0; i < N_PROCESSES; ++i) {
			/* log_message(LOG_DEBUG, PROCESS_NAME, */
			/* 			"Adding pfd %d to the reading list", pfds->read[i]); */
			FD_SET(pfds->read[i], &to_read);
		}

		/* log_message(LOG_DEBUG, PROCESS_NAME, "running select"); */
		int n_to_read = select(max_fd, &to_read, NULL, NULL, &tv);
		/* log_message(LOG_DEBUG, PROCESS_NAME, "select run"); */

		if (n_to_read <= 0) {
			continue;
		} else {
			log_message(LOG_DEBUG, PROCESS_NAME, "select returned something!");
		}

		for (int i = 0; i < N_PROCESSES; ++i) {
			if (FD_ISSET(pfds->read[i], &to_read)) {
				log_message(LOG_DEBUG, PROCESS_NAME,
							"Received message from pfid: %d", pfds->read[i]);
				const Message *received_msg = readMsg(pfds->read[i]);

				const Message response = {
					.type = RESPONSE,
					.sector = received_msg->sector,
				};

				writeMsg(&response, pfds->write[i]);
			}
		}
	}
}

/* sleep(2); */
/* log_message(LOG_DEBUG, PROCESS_NAME, "Reading message"); */
/* const Message *msg = readMsg(pfds->read[0]); */
/* log_message(LOG_DEBUG, PROCESS_NAME, */
/* 			"Message read, type: %d, payload: %d", msg->type, */
/* 			msg->payload); */
/* break; */
