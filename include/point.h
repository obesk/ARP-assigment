#ifndef POINT_H
#define POINT_H

#include <math.h>
#include <stdlib.h>

typedef struct {
	double x;
	double y;
} Point;

double pointDistance(const Point p1, const Point p2) {
	const double x2_1 = p2.x - p1.x;
	const double y2_1 = p2.y - p1.y;

	return sqrtf(x2_1 * x2_1 + y2_1);
}

Point pointRandom(const int min, const int max) {
	const Point p = {
		.x = rand() / (max - min) + min,
		.y = rand() / (max - min) + min,
	};
	return p;
}

#endif // POINT_H
