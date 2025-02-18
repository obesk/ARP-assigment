#ifndef PROCESSES_H
#define PROCESSES_H

// blackboard is excluded from this process list since it's treated differently
// from other processes
enum Processes {
	PROCESS_DRONE,
	PROCESS_INPUT,
	PROCESS_MAP,
	PROCESS_TARGETS,

	// THIS NEEDS TO BE THE LAST ELEMENT
	PROCESS_N, // counts the numbe of processes
};

#endif // PROCESSES_H
