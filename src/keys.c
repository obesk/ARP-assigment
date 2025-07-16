#include "keys.h"

void keys_direction_init() {
	for (int i = 0; i < CHARS_NUMBER; ++i) {
		DIRECTION_KEYS[i] = -1;
	}

	for (int i = 0; i < DIR_N; ++i) {
		DIRECTION_KEYS[(int)KEYS[i]] = i;
	}
}
