#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include "vec2d.h"
#include "target.h"
#include "obstacle.h"
#include "processes.h"

#include <stdbool.h>

#ifndef PROCESS_NAME
#define PROCESS_NAME "blackboard.h"
#endif // PROCESS_NAME

#define GEOFENCE 100

// the order here is important, do not change !
enum MsgType {
	// sent by processes
	TYPE_GET,
	TYPE_SET,

	// sent by blackboard
	TYPE_RESULT,
	TYPE_DATA,
};

enum Result {
	RES_OK,
	RES_REJ,
	RES_ERR,
};

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

union Payload {
	enum Result ack;
	// here the drone struct is "split" so that different processes that update
	// the force (like input) don't interfer with other processess which update
	// the position (drone)
	struct Vec2D drone_position;
	struct Vec2D drone_force;
	struct Vec2D drone_actual_force;
	struct Targets targets;
	struct Obstacles obstacles;
	struct Config config;
	int score;
};

enum MemorySector {
	SECTOR_DRONE_POSITION,
	SECTOR_DRONE_FORCE,
	SECTOR_DRONE_ACTUAL_FORCE,

	SECTOR_TARGETS,
	SECTOR_OBSTACLES,

	SECTOR_CONFIG,

	SECTOR_SCORE,

	SECTOR_N,
};

struct Message {
	enum MsgType type;
	enum MemorySector sector;
	union Payload payload;
};

// TODO: since they are static they should be uppercase
static const struct Message error_msg = {.type = TYPE_RESULT,
										 .payload.ack = RES_ERR};

static const struct Message reject_msg = {.type = TYPE_RESULT,
										  .payload.ack = RES_REJ};


struct Message messageRead(const int pfd);
bool messageWrite(const struct Message *const m, const int wpfd);
struct Message blackboard_get(enum MemorySector sector, const int wpfd,
							  const int rpfd);
bool message_ok(const struct Message *const m);
bool blackboard_set(enum MemorySector sector, const union Payload *payload,
					int wpfd, int rpfd);
int blackboard_get_score(int wpfd, int rpfd);
struct Vec2D blackboard_get_drone_position(int wpfd, int rpfd);
struct Targets blackboard_get_targets(int wpfd, int rpfd);
struct Vec2D blackboard_get_drone_force(int wpfd, int rpfd);
struct Vec2D blackboard_get_drone_actual_force(int wpfd, int rpfd);
struct Config blackboard_get_config(int wpfd, int rpfd);
struct Obstacles blackboard_get_obstacles(int wpfd, int rpfd);

#endif // BLACKBOARD_H
