#define PROCESS_NAME "DRONE"

#include "blackboard.h"
#include "logging.h"
#include "vec2d.h"

#include <unistd.h>

#define DRONE_MASS 1.0
#define VISCOUS_COEFF 1.0

#define PERIOD 100
#define US_TO_S 0.000001

#define MAX_OBSTACLE_DISTANCE 5.0
#define MAX_TARGET_DISTANCE 5.0
#define TARGET_CAUGHT_DISTANCE 1

// TODO: this needs to be decreased probably
#define TARGET_ATTRACTION_COEFF 50.0

struct Vec2D
calulate_target_attraction_force(const struct Targets targets,
								 const struct Vec2D drone_position);

int main(int argc, char **argv) {
	log_message(LOG_INFO, PROCESS_NAME, "Drone running");

	if (argc != 3) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Incorrect number of arguments, expected: 3, received: %d",
					argc);
		exit(1);
	}

	// TODO: add error check
	int rpfd = atoi(argv[1]);
	int wpfd = atoi(argv[2]);

	struct Vec2D old_drone_positions[2] = {0}; // 0 is the most recent
	long old_time_passed = PERIOD;			   // time passed between the two

	while (1) {
		const struct Message drone_force_answer =
			blackboard_get(SECTOR_DRONE_FORCE, wpfd, rpfd);

		// in case of error in retrieveing the data (it should not happen)
		// 0, 0 force is assumed
		const struct Vec2D drone_force =
			message_ok(&drone_force_answer)
				? drone_force_answer.payload.drone_force
				: (struct Vec2D){0};

		log_message(LOG_DEBUG, PROCESS_NAME, "drone_force: x %f, y: %f",
					drone_force.x, drone_force.y);

		const struct Message position_answer =
			blackboard_get(SECTOR_DRONE_POSITION, wpfd, rpfd);

		// in case of error in retrieveing the data (it should not happen)
		// 0, 0 position is assumed
		const struct Vec2D drone_position =
			message_ok(&position_answer)
				? position_answer.payload.drone_position
				: (struct Vec2D){0};

		// based on this specific implementation the drone should be the
		// only one to update it's position so the position reading is
		// unecessary, it's still better to include the case of it getting
		// updated by another process. In this case we shift the old
		// position so that we consider the last read position as the last
		// position
		long time_passed = PERIOD; // FIXME: put clock here
		if (!vec2D_equals(old_drone_positions[0], drone_position)) {
			old_drone_positions[1] = old_drone_positions[0];
			old_drone_positions[0] = drone_position;

			// TODO: here we are assuming that the position got updated in
			// the middle of this process period to get a proper value it
			// would be necessary to add the update time of the position
			old_time_passed = time_passed / 2;
			time_passed /= 2;
		}

		const struct Message targets_answer =
			blackboard_get(SECTOR_TARGETS, wpfd, rpfd);

		const struct Targets targets = message_ok(&targets_answer)
										   ? targets_answer.payload.targets
										   : (struct Targets){0};

		const struct Vec2D targets_force =
			calulate_target_attraction_force(targets, drone_position);

		const struct Vec2D curr_force = vec2D_sum(drone_force, targets_force);

		// Latex formula :
		// x_i = \frac{2M x_{i-1} - M x_{i-2} + K T_{i-1} x_{i-1} + T_{i-1}
		// T_i \sum F_{x_i}}{M + K T_{i-1}} this formula accounts for the
		// two times beign slightly differnt between the 3 sample data

		// TODO: a function would be better to not have to repeat the
		// formula and to deal with boundaries
		// TODO: obstacle, target and boerder force missing
		struct Vec2D new_position = {
			.x = (2 * DRONE_MASS * old_drone_positions[0].x -
				  DRONE_MASS * old_drone_positions[1].x +
				  VISCOUS_COEFF * (old_time_passed * US_TO_S) *
					  old_drone_positions[0].x +
				  (old_time_passed * US_TO_S) * (time_passed * US_TO_S) *
					  curr_force.x) /
				 (DRONE_MASS + VISCOUS_COEFF * (old_time_passed * US_TO_S)),

			.y = (2 * DRONE_MASS * old_drone_positions[0].y -
				  DRONE_MASS * old_drone_positions[1].y +
				  VISCOUS_COEFF * (old_time_passed * US_TO_S) *
					  old_drone_positions[0].y +
				  (old_time_passed * US_TO_S) * (time_passed * US_TO_S) *
					  curr_force.y) /
				 (DRONE_MASS + VISCOUS_COEFF * (old_time_passed * US_TO_S)),
		};

		const union Payload payload = {.drone_position = new_position};

		blackboard_set(SECTOR_DRONE_POSITION, &payload, wpfd, rpfd);

		// struct Message response =
		// 	blackboard_get(SECTOR_DRONE_POSITION, wpfd, rpfd);

		// checking if drone caught correct target and removing it if caught
		// the targets are in reverse order so that the first target to be
		// caught is removed easily by decreasing the number
		if (targets.n &&
			vec2D_distance(drone_position, targets.targets[targets.n - 1]) <
				TARGET_CAUGHT_DISTANCE) {
			log_message(LOG_INFO, PROCESS_NAME, "user caught target");
			struct Targets new_targets = targets;
			new_targets.n = targets.n - 1;
			blackboard_set(SECTOR_TARGETS,
						   &(union Payload){.targets = new_targets}, wpfd,
						   rpfd);
		}

		old_drone_positions[1] = old_drone_positions[0];
		old_drone_positions[0] = drone_position;

		// FIXME: a more refined sleep
		usleep(PERIOD);
	}

	close(wpfd);
	close(rpfd);
	return 0;
}

struct Vec2D
calulate_target_attraction_force(const struct Targets targets,
								 const struct Vec2D drone_position) {
	// computing targets attraction force
	struct Vec2D targets_force = {0};
	for (int i = 0; i < targets.n; ++i) {
		const double target_drone_distance =
			vec2D_distance(targets.targets[i], drone_position);

		if (target_drone_distance > MAX_TARGET_DISTANCE) {
			continue;
		}

		targets_force = vec2D_sum(
			targets_force,
			vec2D_scalar_mult(-TARGET_ATTRACTION_COEFF,
							  vec2D_diff(drone_position, targets.targets[i])));
	}
	return targets_force;
}
