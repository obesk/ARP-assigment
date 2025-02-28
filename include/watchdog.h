#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "processes.h"

#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#define WATCHDOG_HEARTBEAT SIGRTMIN

bool watchdog_send_hearthbeat(const pid_t watchdog_pid,
							  const enum Processes process) {
	const union sigval value = {.sival_int = process};
	// TODO: error management
	const int ret = sigqueue(watchdog_pid, WATCHDOG_HEARTBEAT, value);
	return ret == 0;
	return true;
}

#endif
