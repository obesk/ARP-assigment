#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include "drone.h"
#include "logging.h"
#include "obstacle.h"
#include "target.h"
#include "vec2d.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PROCESS_NAME
#define PROCESS_NAME "blackboard.h"
#endif // PROCESS_NAME

#define BOARD_SIZE 10

// the order here is important, do not change !
enum MsgType {
	// sent by processes
	GET,
	SET,

	// sent by blackboard
	RESPONSE,
	DATA,
};

enum MemorySector {
	DRONE,
	TARGETS,
	OBSTACLES,
};

enum ACK {
	OK,
	REJ,
	ERR,
};

union Payload {
	enum ACK ack;
	struct Drone drone;
	struct Targets targets;
	struct Obstacles obstacles;
};

struct Message {
	enum MsgType type;
	enum MemorySector sector;
	union Payload payload;
};

static const struct Message error_msg = {.type = RESPONSE, .payload.ack = ERR};

struct Message messageRead(const int pfd) {
	struct Message msg;

	ssize_t bytes_read = read(pfd, &msg, sizeof(struct Message));

	if (bytes_read == -1) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"read_message: Error reading from pipe");
		return error_msg;
	}
	if (bytes_read != sizeof(struct Message)) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"read_message: Partial read detected, bytes read: %d\n",
					bytes_read);
		return error_msg;
	}

	return msg;
}

bool messageWrite(const struct Message *const m, const int wpfd) {
	log_message(LOG_DEBUG, PROCESS_NAME, "trying to write message to pfd: %d",
				wpfd);

	ssize_t bytes_written = write(wpfd, m, sizeof(struct Message));

	if (bytes_written == -1) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"write_message: Error writing to pipe");
		return false;
	}
	if (bytes_written != sizeof(struct Message)) {
		fprintf(stderr, "write_message: Partial write detected\n");
		return false;
	}
	log_message(LOG_DEBUG, PROCESS_NAME, "correctly wrote message to pfd: %d",
				wpfd);

	return true;
}

struct Message messageGet(const struct Message *const msg, const int wpfd,
						  const int rpfd) {
	if (msg->type != GET) {
		return error_msg;
	}
	const bool ok = messageWrite(msg, wpfd);

	if (!ok) {
		return error_msg;
	}

	return messageRead(rpfd);
}

bool messageSet(const struct Message *const msg, int wpfd, int rpfd) {
	if (msg->type != SET) {
		return false;
	}
	const bool ok = messageWrite(msg, wpfd);

	if (!ok) {
		return false;
	}

	struct Message response = messageRead(rpfd);
	return response.type == RESPONSE && response.payload.ack == OK;
}

#endif // BLACKBOARD_H
