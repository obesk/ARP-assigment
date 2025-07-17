#include "keys.h"
#include "logging.h"

char KEYS[DIR_N] = {
	[DIR_UP] = 'w',		   [DIR_UP_LEFT] = 'q',	 [DIR_LEFT] = 'a',
	[DIR_DOWN_LEFT] = 'z', [DIR_DOWN] = 'x',	 [DIR_DOWN_RIGHT] = 'c',
	[DIR_RIGHT] = 'd',	   [DIR_UP_RIGHT] = 'e', [DIR_STOP] = 's',
};

enum Direction DIRECTION_KEYS[CHARS_NUMBER] = { 0 };

void keys_direction_init() {
	log_message(LOG_INFO, "Initializing direction keys of size: %d", CHARS_NUMBER);
	for (int i = 0; i < CHARS_NUMBER; ++i) {
		DIRECTION_KEYS[i] = -1;
	}

	for (int i = 0; i < DIR_N; ++i) {
		const int index = (int)KEYS[i];
		DIRECTION_KEYS[index] = i;
		log_message(LOG_INFO, "index %d (char %c) = %d", index, index, i);
	}
}
