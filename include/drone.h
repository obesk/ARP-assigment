#ifndef DRONE_H
#define DRONE_H

#include "vec2d.h"

struct Drone {
	struct Vec2D position;
	struct Vec2D force;
	struct Vec2D actual_force;
};

#endif // DRONE_H
