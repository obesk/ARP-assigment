#ifndef COORDINATES_H
#define COORDINATES_H

#include <math.h>

typedef struct {
	int x;
	int y;
} Point;

double pointDistance(const Point p1, const Point p2) {
	const double x2_1 = p2.x - p1.x;
	const double y2_1 = p2.y - p1.y;

	return sqrt(x2_1 * x2_1 + y2_1);
}

typedef Point LinearVelocity;

#endif // COORDINATES_H
