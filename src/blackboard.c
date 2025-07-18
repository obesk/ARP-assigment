#include "blackboard.h"
#include "logging.h"

#include <unistd.h>

const struct Message error_msg = {.type = TYPE_RESULT, .payload.ack = RES_ERR};

const struct Message reject_msg = {.type = TYPE_RESULT, .payload.ack = RES_REJ};

struct Message messageRead(const int pfd) {
	struct Message msg;

	ssize_t bytes_read = read(pfd, &msg, sizeof(struct Message));

	if (bytes_read == -1) {
		log_message(LOG_CRITICAL, "read_message: Error reading from pipe");
		return error_msg;
	}
	if (bytes_read != sizeof(struct Message)) {
		log_message(LOG_CRITICAL,
					"read_message: Partial read detected, bytes read: %ld\n",
					bytes_read);
		return error_msg;
	}

	return msg;
}

bool messageWrite(const struct Message *const m, const int wpfd) {
	log_message(LOG_TRACE, "trying to write message to pfd: %d", wpfd);

	ssize_t bytes_written = write(wpfd, m, sizeof(struct Message));

	if (bytes_written == -1) {
		log_message(LOG_CRITICAL, "write_message: Error writing to pipe");
		return false;
	}
	if (bytes_written != sizeof(struct Message)) {
		fprintf(stderr, "write_message: Partial write detected\n");
		return false;
	}
	log_message(LOG_TRACE, "correctly wrote message to pfd: %d", wpfd);

	return true;
}

struct Message blackboard_get(enum MemorySector sector, const int wpfd,
							  const int rpfd) {

	if (sector < 0 || sector >= SECTOR_N) {
		log_message(LOG_CRITICAL, "blackboard_get: invalid sector selected");
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
		log_message(LOG_CRITICAL, "blackboard_get: invalid sector selected");
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

int blackboard_get_score(int wpfd, int rpfd) {
	const struct Message score_answer =
		blackboard_get(SECTOR_SCORE, wpfd, rpfd);
	return message_ok(&score_answer) ? score_answer.payload.score : 0;
}

struct Vec2D blackboard_get_drone_position(int wpfd, int rpfd) {
	const struct Message drone_answer =
		blackboard_get(SECTOR_DRONE_POSITION, wpfd, rpfd);

	return message_ok(&drone_answer) ? drone_answer.payload.drone_position
									 : (struct Vec2D){0};
}

struct Targets blackboard_get_targets(int wpfd, int rpfd) {
	const struct Message targets_answer =
		blackboard_get(SECTOR_TARGETS, wpfd, rpfd);
	return message_ok(&targets_answer) ? targets_answer.payload.targets
									   : (struct Targets){0};
}

struct Vec2D blackboard_get_drone_force(int wpfd, int rpfd) {
	const struct Message answer =
		blackboard_get(SECTOR_DRONE_FORCE, wpfd, rpfd);

	return message_ok(&answer) ? answer.payload.drone_force : (struct Vec2D){0};
}

struct Vec2D blackboard_get_drone_actual_force(int wpfd, int rpfd) {
	const struct Message answer =
		blackboard_get(SECTOR_DRONE_ACTUAL_FORCE, wpfd, rpfd);

	return message_ok(&answer) ? answer.payload.drone_actual_force :
		(struct Vec2D){0};
}

struct Config blackboard_get_config(int wpfd, int rpfd) {
	const struct Message config_answer =
		blackboard_get(SECTOR_CONFIG, wpfd, rpfd);
	return message_ok(&config_answer) ? config_answer.payload.config
									  : (struct Config){0};
}

struct Obstacles blackboard_get_obstacles(int wpfd, int rpfd) {
	const struct Message obstacles_answer =
		blackboard_get(SECTOR_OBSTACLES, wpfd, rpfd);

	return message_ok(&obstacles_answer) ? obstacles_answer.payload.obstacles
										 : (struct Obstacles){0};
}