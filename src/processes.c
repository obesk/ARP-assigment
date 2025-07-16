#include "processes.h"
#include "logging.h"

#include <stdlib.h>

bool process_get_arguments(char **argv, int *const rpfd, int *const wpfd,
		int *const watchdog_pid) { 
	*rpfd = atoi(argv[1]);
	*wpfd = atoi(argv[2]);
	*watchdog_pid = atoi(argv[3]);

	if (!*rpfd || !*wpfd || !*watchdog_pid) {
		log_message(LOG_CRITICAL, 
			"Incorrect arguments passed, expected integers");
		return false;
	}
	return true;
} 