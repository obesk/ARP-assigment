#include "processes.h"
#define PROCESS_NAME "BLACKBOARD"

#include "blackboard.h"
#include "logging.h"
#include "pfds.h"
#include "stdbool.h"

bool messageManage(const Message *const msg, Blackboard *const b, int wpfd);

int main(int argc, char **argv) {
	log_message(LOG_INFO, PROCESS_NAME, "Blackboard running");

	Blackboard blackboard;

	const int expected_argc = PROCESS_N * 2 + 1; //+1 for the program name

	if (argc != expected_argc) {
		log_message(
			LOG_CRITICAL, PROCESS_NAME,
			"Erroneous number of arguments passed, expected: %d, got: %d",
			expected_argc, argc);
	}

	PFDs *pfds = argsToPFDs(&argv[1]);
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
				const Message msg = messageRead(pfds->read[i]);

				messageManage(&msg, &blackboard, pfds->write[i]);
			}
		}
	}
}

bool messageManage(const Message *const msg, Blackboard *const b, int wpfd) {
	Message response;
	response.sector = msg->sector;

	if (msg->type > SET) {
		response = error_msg;
	}

	if (msg->type == GET) {
		response.type = DATA;
		switch (msg->sector) {
		case DRONE_POSITION:
			response.payload.drone_position = b->drone.position;
			break;
		case DRONE_VELOCITY:
			response.payload.drone_velocity = b->drone.velocity;
			break;
		case TARGETS:
			memcpy(response.payload.targets, b->targets,
				   sizeof(int) * MAX_TARGETS);
			break;
		case OBSTACLES:
			memcpy(response.payload.obstacles, b->obstacles,
				   sizeof(int) * MAX_TARGETS);
			break;
		}
	}

	if (msg->type == SET) {
		response.type = RESPONSE;
		response.payload.response = OK;

		switch (msg->sector) {
		case DRONE_POSITION:
			b->drone.position = msg->payload.drone_position;
			break;
		case DRONE_VELOCITY:
			b->drone.velocity = msg->payload.drone_velocity;
			break;
		case TARGETS:
			memcpy(b->targets, msg->payload.targets, sizeof(int) * MAX_TARGETS);
			break;
		case OBSTACLES:
			memcpy(b->obstacles, msg->payload.obstacles,
				   sizeof(int) * MAX_TARGETS);
			break;
		}
	}
	return messageWrite(&response, wpfd);
}
