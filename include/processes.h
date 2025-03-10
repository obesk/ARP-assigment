#ifndef PROCESSES_H
#define PROCESSES_H
// blackboard and watchdog are excluded from this process list since they are
// treated differently from other processes

// FIXME: the blackboard should be considered in some way as it should be
// registerd in the watchdog
enum Processes {
	PROCESS_DRONE,
	PROCESS_INPUT,
	PROCESS_MAP,
	PROCESS_TARGETS,
	PROCESS_OBSTACLES,

	// THIS NEEDS TO BE THE LAST ELEMENT
	PROCESS_N, // counts the number of processes
};

static const long process_periods[PROCESS_N] = {
	[PROCESS_DRONE] = 10000,	 [PROCESS_INPUT] = 20000,
	[PROCESS_MAP] = 100000,		 [PROCESS_TARGETS] = 10000,
	[PROCESS_OBSTACLES] = 10000,
};

#endif // PROCESSES_H
