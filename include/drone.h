#ifndef DRONE_H
#define DRONE_H

enum Coordinates {
	X,
	Y,
};

typedef struct {
	float positions[2];
	float velocities[2];
	float forces[2];
} Drone;

#endif // DRONE_H
