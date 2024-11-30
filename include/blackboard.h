#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include "drone.h"
#include "logging.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PROCESS_NAME
#define PROCESS_NAME "blackboard.h"
#endif // PROCESS_NAME

typedef struct {
	Drone drone;
} Blackboard;

typedef enum { READ, WRITE, RESPONSE } MsgType;

typedef enum {
	DRONE_COORDINATES,
	DRONE_VELOCITIES,
	DRONE_FORCES,
} MemorySector;

typedef enum {
	ACK,
	REJ,
} Response;

typedef struct {
	MsgType type;
	MemorySector sector;

	union {
		float drone_coordinates[2];
		float drone_velocities[2];
		Response response;
	} payload;
} Message;

const Message *readMsg(int pfd) {
	Message *m = malloc(sizeof(Message));
	if (!m) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Memory allocation error while initializing a new message");
	}

	ssize_t bytes_read = read(pfd, m, sizeof(Message));

	if (bytes_read == -1) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"read_message: Error reading from pipe");
		return NULL;
	} else if (bytes_read != sizeof(Message)) {
		fprintf(stderr, "read_message: Partial read detected\n");
		return NULL;
	}

	return m;
}

bool writeMsg(const Message *m, int pfd) {
	ssize_t bytes_written = write(pfd, m, sizeof(Message));

	if (bytes_written == -1) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"write_message: Error writing to pipe");
		return false;
	} else if (bytes_written != sizeof(Message)) {
		fprintf(stderr, "write_message: Partial write detected\n");
		return false;
	}

	return true;
}

#endif // BLACKBOARD_H
