#ifndef POINT_H
#define POINT_H

#include <stdbool.h>

struct Vec2D {
	double x;
	double y;
};

bool vec2D_equals(const struct Vec2D v1, const struct Vec2D v2);
struct Vec2D vec2D_diff(const struct Vec2D v1, const struct Vec2D v2);
double vec2D_modulus(const struct Vec2D v);
double vec2D_distance(const struct Vec2D v1, const struct Vec2D v2);
struct Vec2D vec2D_random(const int min, const int max);
struct Vec2D vec2D_scalar_mult(const double scalar, const struct Vec2D v);
struct Vec2D vec2D_sum(const struct Vec2D v1, const struct Vec2D v2);
struct Vec2D vec2D_normalize(const struct Vec2D v);

#endif // POINT_H
