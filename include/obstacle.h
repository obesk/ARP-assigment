#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "vec2d.h"

#define MAX_OBSTACLES 20

struct Obstacles {
	int n;
	struct Vec2D obstacles[MAX_OBSTACLES];
};

#endif // OBSTACLE_H
