#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "vec2d.h"

// TODO: these values probably needs to be changed to something more tought out
#define MAX_OBSTACLES 9

struct Obstacles {
	int n;
	struct Vec2D obstacles[MAX_OBSTACLES];
};

#endif // OBSTACLE_H
