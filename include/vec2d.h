#ifndef POINT_H
#define POINT_H

#include <math.h>
#include <stdlib.h>

struct Vec2D {
	double x;
	double y;
};

double vec2D_distance(const struct Vec2D v1, const struct Vec2D v2) {
	const double x2_1 = v2.x - v1.x;
	const double y2_1 = v2.y - v1.y;

	return sqrtf(x2_1 * x2_1 + y2_1 * y2_1);
}

struct Vec2D vec2D_random(const int min, const int max) {
	return (struct Vec2D){
		.x = rand() / (max - min) + min,
		.y = rand() / (max - min) + min,
	};
}

struct Vec2D Vec2D_scalar(const double scalar, const struct Vec2D v) {
	return (struct Vec2D){
		.x = v.x * scalar,
		.y = v.y * scalar,
	};
}

struct Vec2D Vec2D_sum(const struct Vec2D v1, const struct Vec2D v2) {
	return (struct Vec2D){
		.x = v1.x + v2.x,
		.y = v1.y + v2.y,
	};
}

struct Vec2D Vec2D_diff(const struct Vec2D v1, const struct Vec2D v2) {
	return (struct Vec2D){
		.x = v1.x - v2.x,
		.y = v1.y - v2.y,
	};
}

#endif // POINT_H
