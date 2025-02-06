#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include "drone.h"
#include "logging.h"
#include "obstacle.h"
#include "point.h"
#include "target.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PROCESS_NAME
#define PROCESS_NAME "blackboard.h"
#endif // PROCESS_NAME

// TODO: these values probably needs to be changed to something more tought out
#define MAX_TARGETS 9
#define MAX_OBSTACLES 9

#define BOARD_SIZE 10

typedef struct {
	Drone drone;
	int num_targets;
	Target targets[MAX_TARGETS];
	int num_ostacles;
	Obstacle obstacles[MAX_OBSTACLES];
} Blackboard;

// the order here is important, do not change !
typedef enum {
	// sent by processes
	GET,
	SET,

	// sent by blackboard
	RESPONSE,
	DATA,
} MsgType;

typedef enum {
	DRONE_POSITION,
	DRONE_VELOCITY,
	TARGETS,
	OBSTACLES,
} MemorySector;

typedef enum {
	OK,
	REJ,
	ERR,
} Response;

typedef union {
	Point drone_position;
	LinearVelocity drone_velocity;
	Response response;
	Target targets[MAX_TARGETS];
	Obstacle obstacles[MAX_OBSTACLES];
} Payload;

typedef struct {
	MsgType type;
	MemorySector sector;
	Payload payload;
} Message;

static const Message error_msg = {.type = RESPONSE, .payload.response = ERR};

Message messageRead(const int pfd) {
	Message msg;

	ssize_t bytes_read = read(pfd, &msg, sizeof(Message));

	if (bytes_read == -1) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"read_message: Error reading from pipe");
		return error_msg;
	}
	if (bytes_read != sizeof(Message)) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"read_message: Partial read detected, bytes read: %d\n",
					bytes_read);
		return error_msg;
	}

	return msg;
}

bool messageWrite(const Message *const m, const int wpfd) {
	log_message(LOG_DEBUG, PROCESS_NAME, "trying to write message to pfd: %d",
				wpfd);

	ssize_t bytes_written = write(wpfd, m, sizeof(Message));

	if (bytes_written == -1) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"write_message: Error writing to pipe");
		return false;
	}
	if (bytes_written != sizeof(Message)) {
		fprintf(stderr, "write_message: Partial write detected\n");
		return false;
	}
	log_message(LOG_DEBUG, PROCESS_NAME, "correctly wrote message to pfd: %d",
				wpfd);

	return true;
}

Message messageGet(const Message *const msg, const int wpfd, const int rpfd) {
	if (msg->type != GET) {
		return error_msg;
	}
	const bool ok = messageWrite(msg, wpfd);

	if (!ok) {
		return error_msg;
	}

	return messageRead(rpfd);
}

bool messageSet(const Message *const msg, int wpfd, int rpfd) {
	if (msg->type != SET) {
		return false;
	}
	const bool ok = messageWrite(msg, wpfd);

	if (!ok) {
		return false;
	}

	Message response = messageRead(rpfd);
	return response.type == RESPONSE && response.payload.response == OK;
}

#endif // BLACKBOARD_H
