#ifndef KEYS_H
#define KEYS_H

#define CHARS_NUMBER (1 << sizeof(char))

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

static char KEYS[DIR_N] = {
	[DIR_UP] = 'w',		   [DIR_UP_LEFT] = 'q',	 [DIR_LEFT] = 'a',
	[DIR_DOWN_LEFT] = 'z', [DIR_DOWN] = 'x',	 [DIR_DOWN_RIGHT] = 'c',
	[DIR_RIGHT] = 'd',	   [DIR_UP_RIGHT] = 'e', [DIR_STOP] = 's',
};

// doing an array this way it's useful to remove the use of switches which only
// work with constant expressions, so not compatible if the keymap is loaded at
// runtime
// it needs to be able to contain all possible chars
static enum Direction DIRECTION_KEYS[CHARS_NUMBER];

void keys_direction_init() {
	for (int i = 0; i < CHARS_NUMBER; ++i) {
		DIRECTION_KEYS[i] = -1;
	}

	for (int i = 0; i < DIR_N; ++i) {
		DIRECTION_KEYS[(int)KEYS[i]] = i;
	}
}

#endif // KEYS_H