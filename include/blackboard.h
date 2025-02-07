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
	TYPE_GET,
	TYPE_SET,

	// sent by blackboard
	TYPE_RESULT,
	TYPE_DATA,
};

enum Result {
	RES_OK,
	RES_REJ,
	RES_ERR,
};

union Payload {
	enum Result ack;
	// here the drone struct is "split" so that differnt processes that update
	// the force (like input) don't interfer with other processess which update
	// the position (drone)
	struct Vec2D drone_position;
	struct Vec2D drone_force;
	struct Targets targets;
	struct Obstacles obstacles;
};

enum MemorySector {
	SECTOR_DRONE_POSITION,
	SECTOR_DRONE_FORCE,

	SECTOR_TARGETS,
	SECTOR_OBSTACLES,

	SECTOR_N,
};

struct Message {
	enum MsgType type;
	enum MemorySector sector;
	union Payload payload;
};

// TODO: since they are static they should be uppercase
static const struct Message error_msg = {.type = TYPE_RESULT,
										 .payload.ack = RES_ERR};

static const struct Message reject_msg = {.type = TYPE_RESULT,
										  .payload.ack = RES_REJ};

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

struct Message blackboard_get(enum MemorySector sector, const int wpfd,
							  const int rpfd) {

	if (sector < 0 || sector >= SECTOR_N) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"blackboard_get: invalid sector selected");
		return error_msg;
	}

	const struct Message m = {
		.type = TYPE_GET,
		.sector = sector,
	};

	const bool ok = messageWrite(&m, wpfd);

	if (!ok) {
		return error_msg;
	}
	const struct Message answer = messageRead(rpfd);
	// it doesn't make sense to receive a messsage of type TYPE_RESULT after a
	// get
	if (answer.type != TYPE_DATA) {
		return error_msg;
	}

	return answer;
}

bool message_ok(const struct Message *const m) {
	return (m->type != TYPE_RESULT || m->payload.ack == RES_OK);
}

bool blackboard_set(enum MemorySector sector, const union Payload *payload,
					int wpfd, int rpfd) {

	if (sector < 0 || sector >= SECTOR_N) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"blackboard_get: invalid sector selected");
		return false;
	}

	const struct Message m = {
		.type = TYPE_SET,
		.sector = sector,
		.payload = *payload,
	};

	const bool ok = messageWrite(&m, wpfd);

	if (!ok) {
		return false;
	}

	struct Message response = messageRead(rpfd);
	return response.type == TYPE_RESULT && response.payload.ack == RES_OK;
}

#endif // BLACKBOARD_H
