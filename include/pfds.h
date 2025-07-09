#ifndef PIDS_H
#define PIDS_H

#include "logging.h"
#include "processes.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PROCESS_NAME
#define PROCESS_NAME "pids.h"
#endif // PROCESS_NAME

#define INT_STR_LEN 11 // Enough to store INT_MAX (10 digits) + null terminator

struct PFDs {
	int read[PROCESS_N];  // fds to receive data from the processes
	int write[PROCESS_N]; // fds to send data to the processes
};

// TODO: fixed process name on logs
// TODO: switch from pointer return to return parameter

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
			log_message(LOG_CRITICAL, PROCESS_NAME,
						"Error while genearating PFD to talk to the process");
			return false;
		}
		blackboard_pfds->read[i] = to_blackboard_pfds[0];
		processes_pfds->write[i] = to_blackboard_pfds[1];
		log_message(LOG_DEBUG, PROCESS_NAME,
					"Generated PFD to write to blackboard: read (blackboard): "
					"%d, write "
					"(process): %d ",
					to_blackboard_pfds[0], to_blackboard_pfds[1]);

		// creating the processes struct PFDs to talk to the blackboard
		const int res2 = pipe(to_process_pfds);
		if (res2 <= -1) {
			log_message(
				LOG_CRITICAL, PROCESS_NAME,
				"Error while genearating PFD to talk to the blackboard");
			return NULL;
		}
		processes_pfds->read[i] = to_process_pfds[0];
		blackboard_pfds->write[i] = to_process_pfds[1];
		log_message(
			LOG_DEBUG, PROCESS_NAME,
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

// TODO: find better name for this functions to better distinguish theme
// char **allPFDsToArgs(const struct PFDs *pfds, const char *program_name) {
// 	const int n_args = (PROCESS_N * 2 + 2);
//
// 	// 2 file descriptors per process + prorgram name + NULL (required by execv)
// 	char **args = malloc(sizeof(char *) * n_args);
//
// 	// +1 to account for the terminator
// 	args[0] = malloc(strlen(program_name) + 1);
// 	strcpy(args[0], program_name);
//
// 	int *p = (int *)pfds;
// 	for (int i = 1; i < n_args - 1; ++i) {
// 		args[i] = malloc(INT_STR_LEN);
// 		snprintf(args[i], INT_STR_LEN, "%d", *p);
// 		log_message(LOG_DEBUG, PROCESS_NAME, "FD[%d]: %s", i, args[i]);
// 		++p;
// 	}
//
// 	args[n_args - 1] = NULL; // setting the last element to NULL for execv
// 	return args;
// }

// char **PFDsToArgs(int read_pfd, int write_pfd, const char *program_name) {
// 	const int n_args = 4; // 1 for the program name + 2 for the pfds + 1 for the
// 						  // NULL required by execv
// 	char **args = malloc((sizeof(char *) * n_args));
//
// 	args[0] = malloc(sizeof(program_name));
// 	strcpy(args[0], program_name);
//
// 	args[1] = malloc(INT_STR_LEN);
// 	args[2] = malloc(INT_STR_LEN);
//
// 	snprintf(args[1], INT_STR_LEN, "%d", read_pfd);
// 	snprintf(args[2], INT_STR_LEN, "%d", write_pfd);
//
// 	args[n_args - 1] = NULL;
// 	return args;
// }

struct PFDs *argsToPFDs(char **argv) {
	struct PFDs *pfds = malloc(sizeof(struct PFDs));

	int *p = (int *)pfds;
	for (int i = 0; i < PROCESS_N * 2; ++i) {
		*p = atoi(argv[i]);
		log_message(LOG_DEBUG, PROCESS_NAME, "Parsed FD[%d]: %d", i, *p);
		++p;
	}
	return pfds;
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

#endif // PIDS_H
