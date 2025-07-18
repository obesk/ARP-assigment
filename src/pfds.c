#include "logging.h"
#include "processes.h"
#include "pfds.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

bool newPFDs(struct PFDs *const blackboard_pfds,
			 struct PFDs *const processes_pfds) {
	if (!blackboard_pfds || !processes_pfds) {
		return false;
	}

	int to_blackboard_pfds[2];
	int to_process_pfds[2];

	for (int i = 0; i < PROCESS_N; i++) {
		// creating the blackboard struct PFDs to talk to the processes
		const int res = pipe(to_blackboard_pfds);
		if (res <= -1) {
			log_message(LOG_CRITICAL,
						"Error while genearating PFD to talk to the process");
			return false;
		}
		blackboard_pfds->read[i] = to_blackboard_pfds[0];
		processes_pfds->write[i] = to_blackboard_pfds[1];
		log_message(LOG_DEBUG,
					"Generated PFD to write to blackboard: read (blackboard): "
					"%d, write "
					"(process): %d ",
					to_blackboard_pfds[0], to_blackboard_pfds[1]);

		// creating the processes struct PFDs to talk to the blackboard
		const int res2 = pipe(to_process_pfds);
		if (res2 <= -1) {
			log_message(LOG_CRITICAL,
				"Error while genearating PFD to talk to the blackboard");
			return NULL;
		}
		processes_pfds->read[i] = to_process_pfds[0];
		blackboard_pfds->write[i] = to_process_pfds[1];
		log_message(LOG_DEBUG,
			"Generated PFD to write to process: read (process): %d, write "
			"(blackboard): %d ",
			to_process_pfds[0], to_process_pfds[1]);
	}
	return true;
}

void closeAllPFDs(struct PFDs *const pfds) {
	for (int i = 0; i < PROCESS_N; ++i) {
		close(pfds->read[i]);
		close(pfds->write[i]);
	}
}

void argsToPFDs(struct PFDs *const pfds, char **argv) {
	int *p = (int *)pfds;
	for (int i = 0; i < PROCESS_N * 2; ++i) {
		*p = atoi(argv[i]);
		log_message(LOG_DEBUG, "Parsed FD[%d]: %d", i, *p);
		++p;
	}
}

int getMaxFd(const struct PFDs *const pfds) {
	if (PROCESS_N <= 0) {
		return -1;
	}

	int max_fd = pfds->read[0];
	for (int i = 1; i < PROCESS_N; ++i) {
		if (pfds->read[i] > max_fd) {
			max_fd = pfds->read[i];
		}
	}
	return max_fd;
}
