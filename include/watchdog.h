#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "processes.h"

#include <stdbool.h>
#include <unistd.h>

#ifndef PROCESS_NAME
#define PROCESS_NAME "watchdog.h"
#endif // PROCESS_NAME

#define WATCHDOG_HEARTBEAT SIGRTMIN

bool watchdog_send_hearthbeat(const pid_t watchdog_pid,
							  const enum Processes process);

void watchdog_stop_process(int signum);

void watchdog_register_term_handler();


#endif //WATCHDOG_H
