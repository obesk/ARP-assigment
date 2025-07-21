#include "config.h"

#include "logging.h"
#include "obstacle.h"
#include "target.h"

#include <cjson/cJSON.h>

#include <string.h>
#include <stdlib.h>

#define JSON_MAX_FILE_SIZE 1000

bool read_ip(int *ip_addr, const cJSON *ip_blocks);

int loadJSONConfig(struct Config *const c) {
	FILE *file;
	char jsonBuffer[JSON_MAX_FILE_SIZE];

	file = fopen("appsettings.json", "r");

	log_message(LOG_INFO, "Opened file");

	if (file == NULL) {
		perror("Error opening the file");
		return EXIT_FAILURE;
	}

	int len = fread(jsonBuffer, 1, sizeof(jsonBuffer), file);
	if (!len) {
		perror("Error reading file");
	}
	log_message(LOG_INFO, "Read file");

	cJSON *json = cJSON_Parse(jsonBuffer);
	log_message(LOG_INFO, "Parsed json");

	if (json == NULL) {
		perror("Error parsing the file");
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

	read_ip(c->publisher_ip,
		 cJSON_GetObjectItemCaseSensitive(json, "publisher_ip"));
	c->publisher_port = cJSON_GetObjectItemCaseSensitive(json,
		 "publisher_port")->valueint;

	read_ip(c->subscriber_ip,
		 cJSON_GetObjectItemCaseSensitive(json, "subscriber_ip"));
	c->subscriber_port = cJSON_GetObjectItemCaseSensitive(json,
			 "subscriber_port")->valueint;

	read_ip(c->publisher_server_ip,
		 cJSON_GetObjectItemCaseSensitive(json, "publisher_server_ip"));
	c->publisher_server_port = cJSON_GetObjectItemCaseSensitive(json,
		 "publisher_server_port")->valueint;

	log_message(LOG_INFO, "read values");

	fclose(file);
	return true;
}

bool read_ip(int *ip_addr, const cJSON *ip_blocks) {
	const cJSON *ip_block;
	int *iter = ip_addr;
	cJSON_ArrayForEach(ip_block, ip_blocks) {
		*(iter++) = ip_block->valueint;
	}
	return true;
}