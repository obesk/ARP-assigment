#ifndef PIDS_H
#define PIDS_H

#include "logging.h"
#include "processes.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PROCESS_NAME
#define PROCESS_NAME "pids.h"
#endif // PROCESS_NAME

#define INT_STR_LEN (log10(INT_MAX) + 1)

typedef struct {
	int read[N_PROCESSES];	// fds to receive data from the processes
	int write[N_PROCESSES]; // fds to send data to the processes
} PFDs;

// TODO: fixed process name on logs

PFDs *newPFDs(void) {
	PFDs *pfds = malloc(sizeof(PFDs));
	if (!pfds) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Memory allocation error while generating PFDs");
		return NULL;
	}

	int tmp_fds[2];
	for (int i = 0; i < N_PROCESSES; i++) {
		const int res = pipe(tmp_fds);
		if (res <= -1) {
			log_message(LOG_CRITICAL, PROCESS_NAME,
						"Error while genearating PID");
			return NULL;
		}
		pfds->read[i] = tmp_fds[0];
		pfds->write[i] = tmp_fds[1];
	}
	return pfds;
}

char **PFDsToArgs(const PFDs *pfds, const char *program_name) {
	char **args =
		malloc(N_PROCESSES * 2 + 2); // 2 file descriptors per process +
									 // prorgram name + NULL (required by execv)

	args[0] =
		malloc(strlen(program_name) + 1); // +1 to account for the terminator
	strcpy(args[0], program_name);

	int *p = (int *)pfds;
	for (int i = 1; i <= N_PROCESSES * 2; ++i) {
		args[i] = malloc(INT_STR_LEN);
		snprintf(args[i], INT_STR_LEN, "%d", *p);
		log_message(LOG_DEBUG, PROCESS_NAME, "FD[%d]: %s", i, args[i]);
		++p;
	}

	args[N_PROCESSES * 2 + 1] = NULL;
	return args;
}

PFDs *argsToPFDs(char **argv) {
	PFDs *pfds = malloc(sizeof(PFDs));
	int *p = (int *)pfds;
	for (int i = 0; i < N_PROCESSES * 2; i++) {
		*p = atoi(argv[i]);
		log_message(LOG_DEBUG, PROCESS_NAME, "Parsed FD[%d]: %d", i, *p);
		++p;
	}
	return pfds;
}

int getMaxFd(const PFDs *const pfds) {
	int max_fd = pfds->read[0];
	for (int i = 0; i < N_PROCESSES; ++i) {
		if (pfds->read[i] > max_fd) {
			max_fd = pfds->read[i];
		}
	}
	return max_fd;
}

#endif // PIDS_H
