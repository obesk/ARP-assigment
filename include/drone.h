#ifndef DRONE_H
#define DRONE_H

#include "point.h"

typedef Point LinearVelocity;

typedef struct {
	Point position;
	LinearVelocity velocity;
} Drone;

#endif // DRONE_H
