#ifndef LOGGING_H
#define LOGGING_H

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

// Log Levels
enum LogLevels {
	LOG_TRACE,
	LOG_DEBUG,
	LOG_INFO,
	LOG_WARN,
	LOG_ERROR,
	LOG_CRITICAL,

	LOG_N,
};

#define LOG_LEVEL LOG_DEBUG

static const char *LEVEL_NAME[LOG_N] = {
	[LOG_TRACE] = "TRACE", [LOG_DEBUG] = "DEBUG", [LOG_INFO] = "INFO",
	[LOG_WARN] = "WARN",   [LOG_ERROR] = "ERROR", [LOG_CRITICAL] = "CRITICAL",
};

// Shared logging function
// FIXME: seconds print
void log_message(enum LogLevels level, const char *process, const char *format,
				 ...) {
	if (level < LOG_LEVEL) {
		return;
	}

	FILE *logfile = fopen("simulation.log", "a");
	if (!logfile) {
		perror("Error opening log file");
		return;
	}

	const char date_format[] = "%Y-%m-%d %H:%M:%S";
	time_t now = time(NULL);
	char timestamp[20];
	strftime(timestamp, sizeof(timestamp), date_format, localtime(&now));

	va_list args;
	va_start(args, format);
	fprintf(logfile, "[%s] [%s] [%s] ", timestamp, process, LEVEL_NAME[level]);
	vfprintf(logfile, format, args);
	fprintf(logfile, "\n");
	va_end(args);

	fclose(logfile);
}

#endif // LOGGING_H
