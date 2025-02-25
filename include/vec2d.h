#ifndef POINT_H
#define POINT_H

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

struct Vec2D {
	double x;
	double y;
};

bool vec2D_equals(const struct Vec2D v1, const struct Vec2D v2) {
	return (v1.x == v2.x && v1.y == v2.y);
}

struct Vec2D vec2D_diff(const struct Vec2D v1, const struct Vec2D v2) {
	return (struct Vec2D){
		.x = v1.x - v2.x,
		.y = v1.y - v2.y,
	};
}

double vec2D_modulus(const struct Vec2D v) {
	return sqrtf(v.x * v.x + v.y * v.y);
}

double vec2D_distance(const struct Vec2D v1, const struct Vec2D v2) {
	return vec2D_modulus(vec2D_diff(v1, v2));
}

struct Vec2D vec2D_random(const int min, const int max) {
	return (struct Vec2D){
		.x = rand() % (max - min) + min,
		.y = rand() % (max - min) + min,
	};
}

struct Vec2D vec2D_scalar_mult(const double scalar, const struct Vec2D v) {
	return (struct Vec2D){
		.x = v.x * scalar,
		.y = v.y * scalar,
	};
}

struct Vec2D vec2D_sum(const struct Vec2D v1, const struct Vec2D v2) {
	return (struct Vec2D){
		.x = v1.x + v2.x,
		.y = v1.y + v2.y,
	};
}

struct Vec2D vec2D_normalize(const struct Vec2D v) {
	return vec2D_scalar_mult(1.0 / vec2D_modulus(v), v);
}

#endif // POINT_H
