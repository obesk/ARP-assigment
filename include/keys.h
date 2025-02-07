#ifndef KEYS_H
#define KEYS_H

// M is put before the name to
enum Directions {
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

// TODO: this enum could be substituted by loading the keys from a json file
enum KeyValues {
	C_KEY_UP = 'w',
	C_KEY_UP_LEFT = 'q',
	C_KEY_LEFT = 'a',
	C_KEY_DOWN_LEFT = 'z',
	C_KEY_DOWN = 'x',
	C_KEY_DOWN_RIGHT = 'c',
	C_KEY_RIGHT = 'd',
	C_KEY_UP_RIGHT = 'e',
	C_KEY_STOP = 's',

	C_KEY_QUIT = 'q',

	C_KEY_N,
};

static char KEYS[DIR_N] = {
	[DIR_UP] = C_KEY_UP,	   [DIR_UP_LEFT] = C_KEY_UP_LEFT,
	[DIR_LEFT] = C_KEY_LEFT,   [DIR_DOWN_LEFT] = C_KEY_DOWN_LEFT,
	[DIR_DOWN] = C_KEY_DOWN,   [DIR_DOWN_RIGHT] = C_KEY_DOWN_RIGHT,
	[DIR_RIGHT] = C_KEY_RIGHT, [DIR_UP_RIGHT] = C_KEY_UP_RIGHT,
	[DIR_STOP] = C_KEY_STOP,
};

#endif // KEYS_H