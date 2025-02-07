#ifndef TARGET_H
#define TARGET_H

#include "vec2d.h"

#define MAX_TARGETS 9

struct Targets {
	int n;
	struct Vec2D targets[MAX_TARGETS];
};

#endif // TARGET_H
