#define PROCESS_NAME "BLACKBOARD"

#include "drone.h"
#include "obstacle.h"
#include "target.h"
#include "watchdog.h"
#include "time_management.h"
#include "blackboard.h"
#include "logging.h"
#include "pfds.h"

#include "blackboard_publisher_cif.h"
#include "blackboard_subscriber_cif.h"

#include <stdbool.h>
#include <string.h>

#include <cjson/cJSON.h>
#include <stdlib.h>

#define PERIOD process_periods[PROCESS_BLACKBOARD]
// TODO: this needs to be dimensioned correctly
#define JSON_MAX_FILE_SIZE 1000

struct Blackboard {
	struct Drone drone;
	struct Targets targets;
	struct Obstacles obstacles;
	struct Config config;
	int score;
};

struct Message messageManage(const struct Message *const msg,
							struct Blackboard *const b);

int loadJSONConfig(struct Config *const c);

int main(int argc, char **argv) {
	log_message(LOG_INFO, "Blackboard running");

	BPubHandle DDS_blackboard_publisher = blackboard_publisher_create();
	blackboard_publisher_init(DDS_blackboard_publisher);

	BSubHandle DDS_blackboard_subscriber = blackboard_subscriber_create();
	blackboard_subscriber_init(DDS_blackboard_subscriber);

	watchdog_register_term_handler();

	struct Blackboard blackboard = {
		.drone.position.x = GEOFENCE / 2.,
		.drone.position.y = GEOFENCE / 2.,
	};
	loadJSONConfig(&blackboard.config);

	log_message(LOG_INFO, "loaded config");

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

		if (blackboard_subscriber_has_message(DDS_blackboard_subscriber)) {
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

int loadJSONConfig(struct Config *const c) {
	FILE *file;
	char jsonBuffer[JSON_MAX_FILE_SIZE];

	file = fopen("appsettings.json", "r");

	log_message(LOG_INFO, "Opened file");

	if (file == NULL) {
		log_message(LOG_ERROR, "Error opening the file");
		return EXIT_FAILURE;
	}

	int len = fread(jsonBuffer, 1, sizeof(jsonBuffer), file);
	if (!len) {
		log_message(LOG_ERROR, "Error reading the file");
		return EXIT_FAILURE;
	}
	log_message(LOG_INFO, "Read file");

	cJSON *json = cJSON_Parse(jsonBuffer);
	log_message(LOG_INFO, "Parsed json");

	if (json == NULL) {
		log_message(LOG_ERROR, "Error parsing the file");
		return EXIT_FAILURE;
	}

	log_message(LOG_INFO, "No errors in parsing");

	c->n_obstacles =
		cJSON_GetObjectItemCaseSensitive(json, "n_obstacles")->valueint;
	if (c->n_obstacles < 0 || c->n_obstacles > MAX_OBSTACLES) {
		log_message(LOG_ERROR,
					"Value specified for numer of obstacles in settings is not"
					"valid, setting it to %d",
					MAX_OBSTACLES);
	}

	c->n_targets =
		cJSON_GetObjectItemCaseSensitive(json, "n_targets")->valueint;
	if (c->n_targets < 0 || c->n_targets > MAX_TARGETS) {
		log_message(
			LOG_ERROR,
			"Value specified for numer of targets in settings is not valid,"
			"setting it to %d",
			MAX_TARGETS);
	}

	c->force_applied_N =
		cJSON_GetObjectItemCaseSensitive(json, "force_applied_N")->valuedouble;
	c->drone_mass =
		cJSON_GetObjectItemCaseSensitive(json, "drone_mass_kg")->valuedouble;
	c->viscous_coefficient =
		cJSON_GetObjectItemCaseSensitive(json, "viscous_coefficient_Nms")
			->valuedouble;

	c->max_obstacle_distance =
		cJSON_GetObjectItemCaseSensitive(json, "max_obstacle_distance_m")
			->valuedouble;
	c->min_obstacle_distance =
		cJSON_GetObjectItemCaseSensitive(json, "min_obstacle_distance_m")
			->valuedouble;

	c->obstacle_repulsion_coeff =
		cJSON_GetObjectItemCaseSensitive(json, "obstacle_repulsion_coeff")
			->valuedouble;

	c->max_target_distance =
		cJSON_GetObjectItemCaseSensitive(json, "max_target_distance_m")
			->valuedouble;
	c->target_caught_distance =
		cJSON_GetObjectItemCaseSensitive(json, "target_caught_distance_m")
			->valuedouble;
	c->target_attraction_coeff =
		cJSON_GetObjectItemCaseSensitive(json, "target_attraction_coeff")
			->valuedouble;

	//TODO: manage split false/true
	memset(c->active_processes, false, sizeof(bool) * PROCESS_N);
	const cJSON *active_processes = cJSON_GetObjectItemCaseSensitive(json, "active_processes");
	const cJSON *process;

	cJSON_ArrayForEach(process, active_processes) {
		c->active_processes[process->valueint] = true;
	}

	log_message(LOG_INFO, "read values");

	fclose(file);
	return true;
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
