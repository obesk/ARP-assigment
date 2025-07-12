#ifndef SLEEPING_H
#define SLEEPING_H

#include "logging.h"

#include <stdbool.h>
#include <sys/param.h>
#include <time.h>
#include <unistd.h>

#ifndef PROCESS_NAME
#define PROCESS_NAME "time_management.h"
#endif // PROCESS_NAME

#define US_IN_S 1000000
#define NS_IN_US 1000

struct timespec us_to_ts(const long t) {
	return (struct timespec){
		.tv_sec = t / US_IN_S,
		.tv_nsec = (t % US_IN_S) * NS_IN_US,
	};
}

long ts_diff_us(const struct timespec t1, const struct timespec t2) {
	return (t1.tv_sec - t2.tv_sec) * US_IN_S +
		   (t1.tv_nsec - t2.tv_nsec) / NS_IN_US;
}

bool wait_for_next_period(long period, const struct timespec start_exec_ts,
						  const struct timespec end_exec_ts) {
	const long execution_time = ts_diff_us(end_exec_ts, start_exec_ts);

	if (execution_time > period) {
		log_message(LOG_ERROR,
					"execution time: %ld, has exceeded the period of: %ld",
					execution_time, period);
		return false;
	}

	log_message(LOG_DEBUG, "execution time: %ld, period: %ld",
				execution_time, period);
	usleep(period - execution_time);
	return true;
}

#endif
