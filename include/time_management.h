#ifndef SLEEPING_H
#define SLEEPING_H

#include "logging.h"

#include <stdbool.h>

#ifndef PROCESS_NAME
#define PROCESS_NAME "time_management.h"
#endif // PROCESS_NAME

#define US_IN_S 1000000
#define NS_IN_US 1000

struct timespec us_to_ts(const long t);
long ts_diff_us(const struct timespec t1, const struct timespec t2);
bool wait_for_next_period(long period, const struct timespec start_exec_ts,
						  const struct timespec end_exec_ts);

#endif // SLEEPING_H
