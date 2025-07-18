#define PROCESS_NAME "DRONE"

#include "blackboard.h"
#include "logging.h"
#include "processes.h"
#include "time_management.h"
#include "vec2d.h"
#include "watchdog.h"

#include <unistd.h>
#include <stdlib.h>
#include <math.h>

/* #define M 1.0 // drone mass [kg] */
/* #define K 1.0 // viscous koefficient [N * s * m] */

#define PERIOD process_periods[PROCESS_DRONE]

#define PERIODS_CHECK_CONFIG 10

struct Vec2D calculate_target_attraction_force(
	const struct Targets *const targets, const struct Vec2D drone_position,
	double max_target_distance, double target_attraction_coeff);

struct Vec2D calculate_obstacles_repulsion_force(
	const struct Obstacles *const obstacles, const struct Vec2D drone_position,
	double max_obstacle_distance, double min_obstacle_distance,
	double obstacle_repulsion_coeff);

double calculate_drone_position(const double x1, const double x2,
								const double force, const double period,
								const double M, const double K);

int main(int argc, char **argv) {
	log_message(LOG_INFO, "Drone running");

	if (argc < 4) {
		log_message(LOG_CRITICAL,
					"Incorrect number of arguments, expected: 3, received: %d",
					argc);
		exit(1);
	}

	int rpfd, wpfd, watchdog_pid; 
	if (!process_get_arguments(argv, &rpfd, &wpfd, &watchdog_pid)) {
		exit(1);
	}

	watchdog_register_term_handler();

	// FIXME: this should be set by asking to the blackboard
	struct Vec2D old_drone_positions[2] = {
		{GEOFENCE / 2., GEOFENCE / 2.},
		{GEOFENCE / 2., GEOFENCE / 2.},
	};

	struct timespec start_exec_ts, end_exec_ts;

	int update_config_counter = 0;

	struct Config config = blackboard_get_config(wpfd, rpfd);

	while (1) {
		clock_gettime(CLOCK_REALTIME, &start_exec_ts);

		// querying new config every PERIODS_CHECK_CONFIG cycles from
		// blackboard
		if (++update_config_counter >= PERIODS_CHECK_CONFIG) {
			update_config_counter = 0;
			config = blackboard_get_config(wpfd, rpfd);
		}

		const struct Vec2D drone_force = blackboard_get_drone_force(wpfd, rpfd);

		log_message(LOG_INFO, "drone_force: x %lf, y: %lf",
					drone_force.x, drone_force.y);

		const struct Vec2D drone_position = blackboard_get_drone_position(wpfd, rpfd);

		log_message(LOG_INFO, "got drone position");

		// acquiring targets
		const struct Targets targets = blackboard_get_targets(wpfd, rpfd);

		// acquiring obstacles
		const struct Obstacles obstacles = blackboard_get_obstacles(wpfd, rpfd);

		// computing forces
		const struct Vec2D targets_force = calculate_target_attraction_force(
			&targets, drone_position, config.max_target_distance,
			config.target_attraction_coeff);

		const struct Vec2D obstacles_force =
			calculate_obstacles_repulsion_force(
				&obstacles, drone_position, config.max_obstacle_distance,
				config.min_obstacle_distance, config.obstacle_repulsion_coeff);
		log_message(LOG_INFO, "calculated forces");

		const struct Vec2D total_force =
			vec2D_sum(drone_force, vec2D_sum(obstacles_force, targets_force));

		blackboard_set(SECTOR_DRONE_ACTUAL_FORCE, 
				&(union Payload) { .drone_actual_force = total_force },
				wpfd, rpfd);

		log_message(LOG_INFO, "summed forces");

		log_message(LOG_INFO, "total force: x %lf, y: %lf",
					total_force.x, total_force.y);

		// Latex formula :
		// x_i = \frac{2M x_{i-1} - M x_{i-2} + K T_{i-1} x_{i-1} + T_{i-1}
		// T_i \sum F_{x_i}}{M + K T_{i-1}} this formula accounts for the
		// two times beign slightly differnt between the 3 sample data

		struct Vec2D new_position = {
			.x = calculate_drone_position(
				old_drone_positions[0].x, old_drone_positions[1].x,
				total_force.x, (float)PERIOD / US_IN_S, config.drone_mass,
				config.viscous_coefficient),

			.y = calculate_drone_position(
				old_drone_positions[0].y, old_drone_positions[1].y,
				total_force.y, (float)PERIOD / US_IN_S, config.drone_mass,
				config.viscous_coefficient)};

		log_message(LOG_INFO,
			"drone position x: %lf, drone position y: %lf, force x: %lf, "
			"force y: %lf",
			drone_position.x, drone_position.y, total_force.x, total_force.y);

		const union Payload payload = {.drone_position = new_position};

		blackboard_set(SECTOR_DRONE_POSITION, &payload, wpfd, rpfd);

		// struct Message response =
		// 	blackboard_get(SECTOR_DRONE_POSITION, wpfd, rpfd);

		// checking if drone caught correct target and removing it if caught
		// the targets are in reverse order so that the first target to be
		// caught is removed easily by decreasing the number

		log_message(LOG_INFO, "checking if user caught targets");
		const double distance_to_targets =
			vec2D_distance(new_position, targets.targets[targets.n - 1]);
		log_message(LOG_INFO, "targets distance calculated");

		if (targets.n && distance_to_targets < config.target_caught_distance) {
			log_message(LOG_INFO, "user caught target");
			struct Targets new_targets = targets;
			new_targets.n = targets.n - 1;
			blackboard_set(SECTOR_TARGETS,
						   &(union Payload){.targets = new_targets}, wpfd,
						   rpfd);
		}

		old_drone_positions[1] = old_drone_positions[0];
		old_drone_positions[0] = new_position;

		log_message(LOG_INFO,
					"sending hearthbeat from drone, watchdog_pid: %d", watchdog_pid);
		watchdog_send_hearthbeat(watchdog_pid, PROCESS_DRONE);
		clock_gettime(CLOCK_REALTIME, &end_exec_ts);
		wait_for_next_period(PERIOD, start_exec_ts, end_exec_ts);
	}

	close(wpfd);
	close(rpfd);
	return 0;
}

struct Vec2D calculate_target_attraction_force(
	const struct Targets *const targets, const struct Vec2D drone_position,
	double max_target_distance, double target_attraction_coeff) {
	// computing targets attraction force
	struct Vec2D targets_force = {0};

	log_message(LOG_INFO,
		"calculating target attraction force, targets n: %d, target 1: %lf",
		targets->n, targets->targets[0].x);
	for (int i = 0; i < targets->n; ++i) {
		log_message(LOG_INFO, "target added");
		const double target_drone_distance =
			vec2D_distance(targets->targets[i], drone_position);

		if (target_drone_distance > max_target_distance) {
			continue;
		}

		targets_force = vec2D_sum(
			targets_force,
			vec2D_scalar_mult(-target_attraction_coeff,
							  vec2D_diff(drone_position, targets->targets[i])));
	}
	log_message(LOG_INFO,
				"target attraction force calculated x: %lf, y: %lf", 
				targets_force.x, targets_force.y);

	return targets_force;
}

struct Vec2D calculate_obstacle_repulsion_force(
	const struct Vec2D obstacle, const struct Vec2D drone_position,
	double max_obstacle_distance, double min_obstacle_distance,
	double obstacle_repulsion_coeff) {

	const struct Vec2D obstacle_drone_vec =
		vec2D_diff(drone_position, obstacle);

	const double rho =
		fmax(vec2D_modulus(obstacle_drone_vec), min_obstacle_distance);
		// vec2D_modulus(obstacle_drone_vec);

	if (rho > max_obstacle_distance) {
		return (struct Vec2D){0};
	}
	const struct Vec2D grad = vec2D_normalize(obstacle_drone_vec);
	const double force_module = obstacle_repulsion_coeff *
								(1. / rho - 1. / max_obstacle_distance) *
								(1. / (rho * rho));

	return vec2D_scalar_mult(force_module, grad);
}

struct Vec2D calculate_obstacles_repulsion_force(
	const struct Obstacles *const obstacles, const struct Vec2D drone_position,
	double max_obstacle_distance, double min_obstacle_distance,
	double obstacle_repulsion_coeff) {

	struct Vec2D obstacles_force = {0};
	for (int i = 0; i < obstacles->n; ++i) {
		obstacles_force = vec2D_sum(
			obstacles_force,
			calculate_obstacle_repulsion_force(
				obstacles->obstacles[i], drone_position, max_obstacle_distance,
				min_obstacle_distance, obstacle_repulsion_coeff));
	}

	const struct Vec2D walls[4] = {
		{.x = drone_position.x, .y = 0},		// top wall
		{.x = GEOFENCE, .y = drone_position.y}, // right wall
		{.x = drone_position.x, .y = GEOFENCE}, // bottom wall
		{.x = 0, .y = drone_position.y},		// left wall
	};

	for (int i = 0; i < 4; ++i) {
		obstacles_force =
			vec2D_sum(obstacles_force,
					   calculate_obstacle_repulsion_force(
						   walls[i], drone_position, max_obstacle_distance,
						   min_obstacle_distance, obstacle_repulsion_coeff));
	}

	log_message(LOG_INFO,
				"obstacle repulsion force calculated: %lf",
				vec2D_modulus(obstacles_force));
	return obstacles_force;
}

double calculate_drone_position(const double x1, const double x2,
								const double total_force, const double period,
								const double M, const double K) {
	const double numerator =
		2 * M * x1 - M * x2 + K * period * x1 + period * period * total_force;

	const double denominator = M + K * period;
	log_message(LOG_DEBUG,
				"x1 %lf, x2: %lf, force: %lf, period: %lf, numerator: %lf, "
				"denominator: %lf, res: %lf",
				x1, x2, total_force, period, numerator, denominator,
				numerator / denominator);

	return numerator / denominator;
}
