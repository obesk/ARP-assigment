#ifndef KEYS_H
#define KEYS_H

#define CHARS_NUMBER (1 << (sizeof(char) * 8))

// M is put before the name to
enum Direction {
	// order here is important
	// first row
	DIR_UP_LEFT,
	DIR_UP,
	DIR_UP_RIGHT,
	// second row
	DIR_LEFT,
	DIR_STOP,
	DIR_RIGHT,
	// third row
	DIR_DOWN_LEFT,
	DIR_DOWN,
	DIR_DOWN_RIGHT,

	DIR_N,
};

extern char KEYS[DIR_N];
// doing an array this way it's useful to remove the use of switches which only
// work with constant expressions, so not compatible if the keymap is loaded at
// runtime
// it needs to be able to contain all possible chars
extern enum Direction DIRECTION_KEYS[CHARS_NUMBER];

void keys_direction_init();

#endif // KEYS_H