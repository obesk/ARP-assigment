#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "logging.h"
#include "processes.h"

#include <signal.h>
#include <stdbool.h>
#include <unistd.h>

#ifndef PROCESS_NAME
#define PROCESS_NAME "watchdog.h"
#endif // PROCESS_NAME

#define WATCHDOG_HEARTBEAT SIGRTMIN

bool watchdog_send_hearthbeat(const pid_t watchdog_pid,
							  const enum Processes process) {
	const union sigval value = {.sival_int = process};
	// TODO: error management
	const int ret = sigqueue(watchdog_pid, WATCHDOG_HEARTBEAT, value);

	if (ret) {
		log_message(LOG_WARN, PROCESS_NAME,
					"error sending hearthbeat, ret value: %d", ret);
	}

	return ret == 0;
}

#endif
