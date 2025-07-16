#include "watchdog.h"
#include "logging.h"

#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

bool watchdog_send_hearthbeat(const pid_t watchdog_pid,
                                const enum Processes process) {
	const union sigval value = {.sival_int = process};
	// TODO: error management
	const int ret = sigqueue(watchdog_pid, WATCHDOG_HEARTBEAT, value);
	const int errno_received = errno;

	if (ret) {
		log_message(LOG_WARN, "error sending hearthbeat, ret value: %d, errno: %d", ret, errno_received);
		switch(errno_received) {
			case EAGAIN: 
				log_message(LOG_WARN, "EAGAIN");
				break;
			case EINVAL: 
				log_message(LOG_WARN, "EINVAL");
				break;
			case EPERM: 
				log_message(LOG_WARN, "EPERM");
				break;
			case ESRCH: 
				log_message(LOG_WARN, "ESRCH");
				break;
			default:
				log_message(LOG_WARN, "DEFAULT");
		}
	}

	return ret == 0;
}

void watchdog_stop_process(int signum) {
	(void) signum;
	log_message(LOG_INFO, "Process has been instructed to terminate, exiting...");
	exit(0);
}

void watchdog_register_term_handler() {
	// when some processes start failing the pipes break and a SIGPIPE signal is 
	// thrown, SIGPIPE is ignored since it's the watchdog the one which decides 
	// process inactivity and when to stop them
	sigaction(SIGPIPE, &(struct sigaction) { .sa_handler = SIG_IGN }, NULL);
	sigaction(SIGTERM, &(struct sigaction) { 
			.sa_handler = &watchdog_stop_process }, NULL);
}