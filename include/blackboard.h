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
	} else if (bytes_read != sizeof(Message)) {
		fprintf(stderr, "read_message: Partial read detected\n");
		return error_msg;
	}

	return msg;
}

bool messageWrite(const Message *const m, const int wpfd) {
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

bool messageManage(const Message *const msg, Blackboard *const b, int wpfd) {
	Message response;
	response.sector = msg->sector;

	if (msg->type > SET) {
		response = error_msg;
	}

	if (msg->type == GET) {
		response.type = DATA;
		switch (response.sector) {
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

	if (response.type == SET) {
		response.type = RESPONSE;
		response.payload.response = OK;

		switch (response.sector) {
		case DRONE_POSITION:
			b->drone.position = response.payload.drone_position;
			break;
		case DRONE_VELOCITY:
			b->drone.velocity = response.payload.drone_velocity;
			break;
		case TARGETS:
			memcpy(b->targets, response.payload.targets,
				   sizeof(int) * MAX_TARGETS);
			break;
		case OBSTACLES:
			memcpy(b->obstacles, response.payload.obstacles,
				   sizeof(int) * MAX_TARGETS);
			break;
		}
	}
	return messageWrite(&response, wpfd);
}

#endif // BLACKBOARD_H
