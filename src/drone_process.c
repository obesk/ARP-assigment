#include <stdio.h>
#define PROCESS_NAME "DRONE"

#include "blackboard.h"
#include "logging.h"
#include "processes.h"
#include "time_management.h"
#include "vec2d.h"
#include "watchdog.h"

#include <unistd.h>

#define M 1.0 // drone mass [kg]
#define K 1.0 // viscous koefficient [N * s * m]

#define PERIOD process_periods[PROCESS_DRONE]

#define MAX_OBSTACLE_DISTANCE 2
// TODO: consider the possibility of using 1 char distance
#define MIN_OBSTACLE_DISTANCE 1
#define MAX_TARGET_DISTANCE 3.0
#define TARGET_CAUGHT_DISTANCE 1.0

// TODO: this needs to be decreased probably
#define TARGET_ATTRACTION_COEFF 1.0
#define OBSTACLE_REPULSION_COEFF 50.0

struct Vec2D
calculate_target_attraction_force(const struct Targets *const targets,
								  const struct Vec2D drone_position);

struct Vec2D
calculate_obstacle_repulsion_force(const struct Obstacles *const obstacles,
								   const struct Vec2D drone_position);

double calculate_drone_position(const double x1, const double x2,
								const double force, const double period);

int main(int argc, char **argv) {
	log_message(LOG_INFO, PROCESS_NAME, "Drone running");

	if (argc < 4) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Incorrect number of arguments, expected: 3, received: %d",
					argc);
		exit(1);
	}

	// TODO: add error check
	int rpfd = atoi(argv[1]);
	int wpfd = atoi(argv[2]);
	int whatchdog_pid = atoi(argv[3]);

	struct Vec2D old_drone_positions[2] = {
		{GEOFENCE / 2., GEOFENCE / 2.},
		{GEOFENCE / 2., GEOFENCE / 2.},
	}; // 0 is the most recent

	struct timespec start_exec_ts, end_exec_ts;
	while (1) {
		clock_gettime(CLOCK_REALTIME, &start_exec_ts);

		const struct Message drone_force_answer =
			blackboard_get(SECTOR_DRONE_FORCE, wpfd, rpfd);

		// in case of error in retrieveing the data (it should not happen)
		// 0, 0 force is assumed
		const struct Vec2D drone_force =
			message_ok(&drone_force_answer)
				? drone_force_answer.payload.drone_force
				: (struct Vec2D){0};

		log_message(LOG_INFO, PROCESS_NAME, "drone_force: x %lf, y: %lf",
					drone_force.x, drone_force.y);

		const struct Message position_answer =
			blackboard_get(SECTOR_DRONE_POSITION, wpfd, rpfd);

		// in case of error in retrieveing the data (it should not happen)
		// 0, 0 position is assumed
		const struct Vec2D drone_position =
			message_ok(&position_answer)
				? position_answer.payload.drone_position
				: (struct Vec2D){0};

		log_message(LOG_INFO, PROCESS_NAME, "got drone position");

		// TODO: remove this comments if not used
		// based on this specific implementation the drone should be the
		// only one to update it's position so the position reading is
		// unecessary, it's still better to include the case of it getting
		// updated by another process. In this case we shift the old
		// position so that we consider the last read position as the last
		// position
		/* long time_passed = PERIOD; // FIXME: put clock here */
		/* if (!vec2D_equals(old_drone_positions[0], drone_position)) { */
		/* 	old_drone_positions[1] = old_drone_positions[0]; */
		/* 	old_drone_positions[0] = drone_position; */

		// TODO: remove this comments if not used
		// TODO: here we are assuming that the position got updated in
		// the middle of this process period to get a proper value it
		// would be necessary to add the update time of the position
		/* 	old_time_passed = time_passed / 2; */
		/* 	time_passed /= 2; */
		/* } */

		// acquiring targets
		const struct Message targets_answer =
			blackboard_get(SECTOR_TARGETS, wpfd, rpfd);
		const struct Targets targets = message_ok(&targets_answer)
										   ? targets_answer.payload.targets
										   : (struct Targets){0};

		// acquiring obstacles
		const struct Message obstacles_answer =
			blackboard_get(SECTOR_OBSTACLES, wpfd, rpfd);
		const struct Obstacles obstacles =
			message_ok(&obstacles_answer) ? obstacles_answer.payload.obstacles
										  : (struct Obstacles){0};

		// computing forces
		const struct Vec2D targets_force =
			calculate_target_attraction_force(&targets, drone_position);

		const struct Vec2D obstacles_force =
			calculate_obstacle_repulsion_force(&obstacles, drone_position);
		log_message(LOG_INFO, PROCESS_NAME, "calculated forces");

		const struct Vec2D curr_force =
			vec2D_sum(drone_force, vec2D_sum(obstacles_force, targets_force));
		log_message(LOG_INFO, PROCESS_NAME, "summed forces");

		log_message(LOG_INFO, PROCESS_NAME, "total force: x %lf, y: %lf",
					curr_force.x, curr_force.y);

		// Latex formula :
		// x_i = \frac{2M x_{i-1} - M x_{i-2} + K T_{i-1} x_{i-1} + T_{i-1}
		// T_i \sum F_{x_i}}{M + K T_{i-1}} this formula accounts for the
		// two times beign slightly differnt between the 3 sample data

		// TODO: border force missing
		struct Vec2D new_position = {
			.x = calculate_drone_position(
				old_drone_positions[0].x, old_drone_positions[1].x,
				curr_force.x, (float)PERIOD / US_IN_S),
			.y = calculate_drone_position(
				old_drone_positions[0].y, old_drone_positions[1].y,
				curr_force.y, (float)PERIOD / US_IN_S),
		};

		log_message(
			LOG_INFO, PROCESS_NAME,
			"drone position x: %lf, drone position y: %lf, force x: %lf, "
			"force y: %lf",
			drone_position.x, drone_position.y, curr_force.x, curr_force.y);

		const union Payload payload = {.drone_position = new_position};

		blackboard_set(SECTOR_DRONE_POSITION, &payload, wpfd, rpfd);

		// struct Message response =
		// 	blackboard_get(SECTOR_DRONE_POSITION, wpfd, rpfd);

		// checking if drone caught correct target and removing it if caught
		// the targets are in reverse order so that the first target to be
		// caught is removed easily by decreasing the number

		log_message(LOG_INFO, PROCESS_NAME, "checking if user caught targets");
		const double distance_to_targets =
			vec2D_distance(new_position, targets.targets[targets.n - 1]);
		log_message(LOG_INFO, PROCESS_NAME, "targets distance calculated");

		if (targets.n && distance_to_targets < TARGET_CAUGHT_DISTANCE) {
			log_message(LOG_INFO, PROCESS_NAME, "user caught target");
			struct Targets new_targets = targets;
			new_targets.n = targets.n - 1;
			blackboard_set(SECTOR_TARGETS,
						   &(union Payload){.targets = new_targets}, wpfd,
						   rpfd);
		}

		old_drone_positions[1] = old_drone_positions[0];
		old_drone_positions[0] = new_position;

		log_message(LOG_INFO, PROCESS_NAME, "sending hearthbeat");

		watchdog_send_hearthbeat(whatchdog_pid, PROCESS_DRONE);
		clock_gettime(CLOCK_REALTIME, &end_exec_ts);
		log_message(LOG_INFO, PROCESS_NAME, "sleeping");
		wait_for_next_period(PERIOD, start_exec_ts, end_exec_ts);
	}

	close(wpfd);
	close(rpfd);
	return 0;
}

struct Vec2D
calculate_target_attraction_force(const struct Targets *const targets,
								  const struct Vec2D drone_position) {
	// computing targets attraction force
	struct Vec2D targets_force = {0};

	log_message(
		LOG_INFO, PROCESS_NAME,
		"calculating target attraction force, targets n: %d, target 1: %lf",
		targets->n, targets->targets[0].x);
	for (int i = 0; i < targets->n; ++i) {
		log_message(LOG_INFO, PROCESS_NAME, "target added");
		const double target_drone_distance =
			vec2D_distance(targets->targets[i], drone_position);

		if (target_drone_distance > MAX_TARGET_DISTANCE) {
			continue;
		}

		targets_force = vec2D_sum(
			targets_force,
			vec2D_scalar_mult(-TARGET_ATTRACTION_COEFF,
							  vec2D_diff(drone_position, targets->targets[i])));
	}
	log_message(LOG_INFO, PROCESS_NAME,
				"target attraction force calculated: %lf", targets_force);

	return targets_force;
}

struct Vec2D
calculate_obstacle_repulsion_force(const struct Obstacles *const obstacles,
								   const struct Vec2D drone_position) {
	struct Vec2D obstacles_force = {0};
	for (int i = 0; i < obstacles->n; ++i) {
		const struct Vec2D obstacle_drone_vec =
			vec2D_diff(drone_position, obstacles->obstacles[i]);

		const double rho =
			fmax(vec2D_modulus(obstacle_drone_vec), MIN_OBSTACLE_DISTANCE);

		if (rho > MAX_OBSTACLE_DISTANCE) {
			continue;
		}
		const struct Vec2D grad = vec2D_normalize(obstacle_drone_vec);
		const double force_module = OBSTACLE_REPULSION_COEFF *
									(1. / rho - 1. / MAX_OBSTACLE_DISTANCE) *
									(1. / (rho * rho)) * grad.x;
		obstacles_force =
			vec2D_sum(obstacles_force, vec2D_scalar_mult(force_module, grad));
	}

	log_message(LOG_INFO, PROCESS_NAME,
				"obstacle repulsion force calculated: %lf",
				vec2D_modulus(obstacles_force));
	return obstacles_force;
}

double calculate_drone_position(const double x1, const double x2,
								const double force, const double period) {
	const double numerator =
		2 * M * x1 - M * x2 + K * period * x1 + period * period * force;

	const double denominator = M + K * period;
	log_message(LOG_INFO, PROCESS_NAME,
				"x1 %lf, x2: %lf, force: %lf, period: %lf, numerator: %lf, "
				"denominator: %lf, res: %lf",
				x1, x2, force, period, numerator, denominator,
				numerator / denominator);

	return numerator / denominator;
}
