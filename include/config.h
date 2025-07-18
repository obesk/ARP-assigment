#ifndef CONFIG_H 
#define CONFIG_H

#include "processes.h"

#include <stdbool.h>

struct Config {
	int n_obstacles;
	int n_targets;

	float force_applied_N;

	float drone_mass;
	float viscous_coefficient;

	float max_obstacle_distance;
	float min_obstacle_distance;
	float obstacle_repulsion_coeff;

	float max_target_distance;
	float target_caught_distance;
	float target_attraction_coeff;

	bool active_processes[PROCESS_N];
};

int loadJSONConfig(struct Config *const c);

#endif //CONFIG_H
