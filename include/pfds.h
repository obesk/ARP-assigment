#ifndef PIDS_H
#define PIDS_H

#include <stdbool.h>

#ifndef PROCESS_NAME
#define PROCESS_NAME "pids.h"
#endif // PROCESS_NAME

#define INT_STR_LEN 11 // Enough to store INT_MAX (10 digits) + null terminator

struct PFDs {
	int read[PROCESS_N];  // fds to receive data from the processes
	int write[PROCESS_N]; // fds to send data to the processes
	int n_processes;
};

bool newPFDs(struct PFDs *const blackboard_pfds,
	struct PFDs *const processes_pfds);
void closeAllPFDs(struct PFDs *const pfds);
void argsToPFDs(struct PFDs *const pfds, char **argv);
int getMaxFd(const struct PFDs *const pfds);

#endif // PIDS_H
