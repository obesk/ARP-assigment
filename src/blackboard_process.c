#define PROCESS_NAME "BLACKBOARD"

#include "drone.h"
#include "obstacle.h"
#include "target.h"
#include "watchdog.h"
#include "time_management.h"
#include "blackboard.h"
#include "logging.h"
#include "pfds.h"
#include "config.h"

#include "blackboard_publisher_cif.h"
#include "blackboard_subscriber_cif.h"


#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


#define PERIOD process_periods[PROCESS_BLACKBOARD]
// TODO: this needs to be dimensioned correctly

struct Blackboard {
	struct Drone drone;
	struct Targets targets;
	struct Obstacles obstacles;
	struct Config config;
	int score;
};

struct Message messageManage(const struct Message *const msg,
							struct Blackboard *const b);

int main(int argc, char **argv) {
	log_message(LOG_INFO, "Blackboard running");

	struct Blackboard blackboard = {
		.drone.position.x = GEOFENCE / 2.,
		.drone.position.y = GEOFENCE / 2.,
	};

	loadJSONConfig(&blackboard.config);
	log_message(LOG_INFO, "loaded config");

	BPubHandle DDS_blackboard_publisher = blackboard_publisher_create();
	blackboard_publisher_init(DDS_blackboard_publisher,
		blackboard.config.publisher_ip, blackboard.config.publisher_port);

	BSubHandle DDS_blackboard_subscriber = blackboard_subscriber_create();
	blackboard_subscriber_init(DDS_blackboard_subscriber,
		blackboard.config.publisher_server_ip, blackboard.config.subscriber_ip,
		blackboard.config.publisher_server_port, blackboard.config.subscriber_port
	);

	watchdog_register_term_handler();

	// read and write for each processes
	// +1 for the program name and +1 for the watchdog pid
	const int expected_argc = PROCESS_N * 2 + 2;

	if (argc != expected_argc) {
		log_message(
			LOG_CRITICAL,
			"Erroneous number of arguments passed, expected: %d, got: %d",
			expected_argc, argc);
	}

	struct PFDs pfds;
	argsToPFDs(&pfds, &argv[1]);
	int watchdog_pid = atoi(argv[argc - 1]);

	const int max_fd = getMaxFd(&pfds) + 1;
	struct timespec ts_start_exec, ts_end_exec;
	long us_update_config_remaining = PERIOD;

	while (true) {
		clock_gettime(CLOCK_REALTIME, &ts_start_exec);
		fd_set to_read;
		FD_ZERO(&to_read);

		for (int i = 0; i < PROCESS_N; ++i) {
			if (!blackboard.config.active_processes[i]) {
				continue;
			}
			FD_SET(pfds.read[i], &to_read);
		}

		struct timeval select_timeout = {
			.tv_sec = us_update_config_remaining / US_IN_S,
			.tv_usec = us_update_config_remaining % US_IN_S,
		};

		int n_to_read = select(max_fd, &to_read, NULL, NULL, &select_timeout);
		if (n_to_read > 0) {
			log_message(LOG_INFO, "select returned something!");
			for (int i = 0; i < PROCESS_N; ++i) {
				if (blackboard.config.active_processes[i] &&
					FD_ISSET(pfds.read[i], &to_read)) {
					log_message(LOG_INFO, "Received message from pfid: %d of process: %d",
								pfds.read[i], i);
					const struct Message msg = messageRead(pfds.read[i]);

					const struct Message resp_msg =
						 messageManage(&msg, &blackboard);
					messageWrite(&resp_msg, pfds.write[i]);
					if(msg.type == TYPE_SET) {
						blackboard_publish_message(DDS_blackboard_publisher, &msg);
					}
				}
			}
		}

		while (blackboard_subscriber_has_message(DDS_blackboard_subscriber)) {
			log_message(LOG_INFO, "received update message from blackboard");
			const struct Message received_msg = 
				blackboard_subscriber_get_message(DDS_blackboard_subscriber);
			messageManage(&received_msg, &blackboard);
		}

		clock_gettime(CLOCK_REALTIME, &ts_end_exec);
		us_update_config_remaining -= ts_diff_us(ts_end_exec, ts_start_exec);
		if (us_update_config_remaining <= 0) {
			log_message(LOG_INFO,
						"sending hearthbeat from blackboard, watchdog_pid: %d",
						watchdog_pid);
			us_update_config_remaining = PERIOD;
			loadJSONConfig(&blackboard.config);
			watchdog_send_hearthbeat(watchdog_pid, PROCESS_BLACKBOARD);
		}
	}

	blackboard_publisher_free(DDS_blackboard_publisher);
	closeAllPFDs(&pfds);

	return 0;
}

struct Message messageManage(const struct Message *const msg, struct Blackboard *const b) {
	struct Message response;
	response.sector = msg->sector;

	if (msg->type < 0 || msg->type > TYPE_SET) {
		response = error_msg;
	}

	if (msg->type == TYPE_GET) {
		response.type = TYPE_DATA;
		switch (msg->sector) {
		case SECTOR_DRONE_POSITION:
			response.payload.drone_position = b->drone.position;
			break;
		case SECTOR_DRONE_FORCE:
			response.payload.drone_force = b->drone.force;
			break;
		case SECTOR_DRONE_ACTUAL_FORCE:
			response.payload.drone_force = b->drone.actual_force;
			break;
		case SECTOR_TARGETS:
			response.payload.targets = b->targets;
			break;
		case SECTOR_OBSTACLES:
			response.payload.obstacles = b->obstacles;
			break;
		case SECTOR_CONFIG:
			response.payload.config = b->config;
			break;
		case SECTOR_SCORE:
			response.payload.score = b->score;
			break;
		default:
			response = error_msg;
		}
	} else if (msg->type == TYPE_SET) {
		response.type = TYPE_RESULT;
		response.payload.ack = RES_OK;

		switch (msg->sector) {
		case SECTOR_DRONE_POSITION:
			b->drone.position = msg->payload.drone_position;
			break;
		case SECTOR_DRONE_FORCE:
			b->drone.force = msg->payload.drone_force;
			break;
		case SECTOR_DRONE_ACTUAL_FORCE:
			b->drone.actual_force = msg->payload.drone_actual_force;
			break;
		case SECTOR_TARGETS:
			b->targets = msg->payload.targets;
			break;
		case SECTOR_OBSTACLES:
			b->obstacles = msg->payload.obstacles;
			break;
		case SECTOR_SCORE:
			b->score = msg->payload.score;
			break;
		default:
			response = reject_msg;
		}
	}
	return response;
}
