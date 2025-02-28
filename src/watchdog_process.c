#define PROCESS_NAME "WATCHDOG"

#include "logging.h"
#include "processes.h"
#include "stdbool.h"
#include "watchdog.h"

#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define PERIOD 10000
#define US_IN_S 1000000
#define NS_IN_US 1000

// factor to which we multiply the periods to generate the allowed inactive time
#define WAIT_FACTOR 2

// NOTE: the pid is not strictly necessary but prevents other processes to start
// impersonating a process and detect conflicts
struct hearthbeat {
	pid_t pid;
	struct timespec ts;
};

static struct hearthbeat hearthbeats[PROCESS_N];

bool register_hearthbeat(struct hearthbeat hearthbeats[PROCESS_N],
						 const siginfo_t *info);

int main(void) {
	log_message(LOG_INFO, PROCESS_NAME, "Watchdog running");

	// the max period of the processes found to determine the maximum
	// registration time in which all processes should have sent at least an
	// heartbeat;
	// the min period is used to determine the frequency on which to check if
	// the processes are still alive
	long max_period = 0, min_period = process_periods[0];
	for (int i = 0; i < PROCESS_N; ++i) {
		if (process_periods[i] > max_period) {
			max_period = process_periods[i];
		} else if (process_periods[i] < min_period) {
			min_period = process_periods[i];
		}
	}

	sigset_t mask;
	siginfo_t info;
	sigemptyset(&mask);
	sigaddset(&mask, WATCHDOG_HEARTBEAT);
	sigprocmask(SIG_BLOCK, &mask, NULL);

	long wait_time_us = max_period * WAIT_FACTOR;

	struct timespec start_time_ts;
	clock_gettime(CLOCK_REALTIME, &start_time_ts);
	log_message(LOG_INFO, PROCESS_NAME, "starting process registration phase");

	// waiting for all processes to "register"
	while (wait_time_us > 0) {
		log_message(LOG_INFO, PROCESS_NAME,
					"%d us to go for registration phase to finish",
					wait_time_us);

		const struct timespec timeout = {
			.tv_sec = wait_time_us / US_IN_S,
			.tv_nsec = (wait_time_us % US_IN_S) * NS_IN_US,
		};

		log_message(
			LOG_INFO, PROCESS_NAME,
			"waiting for the signals for maximum %d seconds, %d nanoseconds",
			timeout.tv_sec, timeout.tv_nsec);

		int ret = sigtimedwait(&mask, &info, &timeout);

		if (ret >= 0) {
			register_hearthbeat(hearthbeats, &info);
		}

		struct timespec end_time_ts;

		clock_gettime(CLOCK_REALTIME, &end_time_ts);

		wait_time_us -=
			(end_time_ts.tv_sec - start_time_ts.tv_sec) * US_IN_S +
			(end_time_ts.tv_nsec - start_time_ts.tv_nsec) / NS_IN_US;

		start_time_ts = end_time_ts;
	}

	log_message(LOG_INFO, PROCESS_NAME,
				"finished process registration phase, starting process "
				"monitoring phase");

	// check that all processes have correctly registered
	for (int i = 0; i < PROCESS_N; ++i) {
		if (!hearthbeats[i].pid) {
			log_message(
				LOG_CRITICAL, PROCESS_NAME,
				"process %d did not manage to register correctly, exiting...",
				i);
			exit(1);
		}
	}

	// it does not make sense to recheck that all processes are still active
	// after every  received signal; it's enough to check every period of the
	// most frequent process
	long time_from_next_sampling_us = min_period;
	while (true) {
		struct timespec start_time_ts, end_time_ts;

		clock_gettime(CLOCK_REALTIME, &start_time_ts);

		const int ret = sigtimedwait(
			&mask, &info,
			&(struct timespec){
				.tv_sec = time_from_next_sampling_us / US_IN_S,
				.tv_nsec = (time_from_next_sampling_us % US_IN_S) * NS_IN_US,
			});

		if (ret >= 0) {
			register_hearthbeat(hearthbeats, &info);
		}

		clock_gettime(CLOCK_REALTIME, &end_time_ts);

		time_from_next_sampling_us -=
			(end_time_ts.tv_sec - start_time_ts.tv_sec) * US_IN_S +
			(end_time_ts.tv_nsec - start_time_ts.tv_nsec) / NS_IN_US;

		log_message(LOG_INFO, PROCESS_NAME, "time from next sampling %d",
					time_from_next_sampling_us);

		if (time_from_next_sampling_us > 0) {
			continue;
		}

		time_from_next_sampling_us = min_period;

		log_message(LOG_INFO, PROCESS_NAME, "starting check for dead processes",
					time_from_next_sampling_us);

		// checking if some processes are dead
		for (int i = 0; i < PROCESS_N; ++i) {
			const long elapsed_time_from_hearthbeat =
				(end_time_ts.tv_sec - hearthbeats[i].ts.tv_sec) * US_IN_S +
				(end_time_ts.tv_nsec - hearthbeats[i].ts.tv_nsec) / NS_IN_US;

			log_message(LOG_INFO, PROCESS_NAME,
						"elapsed time from hearthbeat for process %d: %d", i,
						elapsed_time_from_hearthbeat);

			if (elapsed_time_from_hearthbeat >
				process_periods[i] * WAIT_FACTOR) {
				log_message(LOG_CRITICAL, PROCESS_NAME,
							"process %d with pid %d has not managed to send a "
							"hearthbeat in the allowed time %d, exiting",
							i, hearthbeats[i].pid,
							process_periods[i] * WAIT_FACTOR);
				// FIXME: here we should kill all other processes
				exit(1);
			}
		}

		log_message(LOG_INFO, PROCESS_NAME, "no dead processes found");
	}
	return 0;
}

bool register_hearthbeat(struct hearthbeat hearthbeats[PROCESS_N],
						 const siginfo_t *info) {

	if (info->si_signo != WATCHDOG_HEARTBEAT) {
		log_message(LOG_WARN, PROCESS_NAME,
					"invalid signal code received: %d from process with pid: "
					"%d, this should not happen, the watchdog should only "
					"receive signals with this code: %d",
					info->si_signo, info->si_pid, WATCHDOG_HEARTBEAT);
		return false;
	}

	const enum Processes sender_process = info->si_int;
	if (sender_process < 0 || sender_process >= PROCESS_N) {
		log_message(
			LOG_ERROR, PROCESS_NAME,
			"received a signal from a process with an invalid number: %d",
			sender_process);
		return false;
	}

	if (hearthbeats[sender_process].pid &&
		hearthbeats[sender_process].pid != info->si_pid) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"a process with pid: %d, tried to impersonate the process "
					"with number: %d, already registered with pid: %d",
					info->si_pid, sender_process,
					hearthbeats[sender_process].pid);
		// FIXME: here all processes should be killed not just exiting
		exit(1);
	}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	hearthbeats[sender_process] = (struct hearthbeat){
		.pid = info->si_pid,
		.ts = ts,
	};

	return true;
}
