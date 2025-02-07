#include "processes.h"
#define PROCESS_NAME "BLACKBOARD"

#include "blackboard.h"
#include "logging.h"
#include "pfds.h"
#include "stdbool.h"

struct Blackboard {
	struct Drone drone;
	struct Targets targets;
	struct Obstacles obstacles;
};

bool messageManage(const struct Message *const msg, struct Blackboard *const b,
				   int wpfd);

int main(int argc, char **argv) {
	log_message(LOG_INFO, PROCESS_NAME, "Blackboard running");

	struct Blackboard blackboard;

	const int expected_argc = PROCESS_N * 2 + 1; //+1 for the program name

	if (argc != expected_argc) {
		log_message(
			LOG_CRITICAL, PROCESS_NAME,
			"Erroneous number of arguments passed, expected: %d, got: %d",
			expected_argc, argc);
	}

	struct PFDs *pfds = argsToPFDs(&argv[1]);
	const int max_fd = getMaxFd(pfds) + 1;

	while (true) {
		fd_set to_read;
		FD_ZERO(&to_read);

		for (int i = 0; i < PROCESS_N; ++i) {
			/* log_message(LOG_DEBUG, PROCESS_NAME, */
			/* 	Adding pfd %d to the reading list", pfds->read[i]); */
			FD_SET(pfds->read[i], &to_read);
		}

		/* log_message(LOG_DEBUG, PROCESS_NAME, "running select"); */
		int n_to_read = select(max_fd, &to_read, NULL, NULL, NULL);
		/* log_message(LOG_DEBUG, PROCESS_NAME, "select run"); */

		if (n_to_read <= 0) {
			continue;
		} else {
			log_message(LOG_DEBUG, PROCESS_NAME, "select returned something!");
		}

		for (int i = 0; i < PROCESS_N; ++i) {
			if (FD_ISSET(pfds->read[i], &to_read)) {
				log_message(LOG_DEBUG, PROCESS_NAME,
							"Received message from pfid: %d", pfds->read[i]);
				const struct Message msg = messageRead(pfds->read[i]);

				messageManage(&msg, &blackboard, pfds->write[i]);
			}
		}
	}
}

bool messageManage(const struct Message *const msg, struct Blackboard *const b,
				   int wpfd) {
	struct Message response;
	response.sector = msg->sector;

	if (msg->type > SET) {
		response = error_msg;
	}

	if (msg->type == GET) {
		response.type = DATA;
		switch (msg->sector) {
		case DRONE:
			response.payload.drone = b->drone;
			break;
		case TARGETS:
			response.payload.targets = b->targets;
			break;
		case OBSTACLES:
			response.payload.obstacles = b->obstacles;
			break;
		}
	}

	if (msg->type == SET) {
		response.type = RESPONSE;
		response.payload.ack = OK;

		switch (msg->sector) {
		case DRONE:
			b->drone = msg->payload.drone;
			break;
		case TARGETS:
			b->targets = msg->payload.targets;
			break;
		case OBSTACLES:
			b->obstacles = msg->payload.obstacles;
			break;
		}
	}
	return messageWrite(&response, wpfd);
}
