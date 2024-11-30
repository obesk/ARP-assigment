#ifndef LOGGING_H
#define LOGGING_H

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

// Log Levels
#define LOG_INFO "INFO"
#define LOG_DEBUG "DEBUG"
#define LOG_WARN "WARN"
#define LOG_ERROR "ERROR"
#define LOG_CRITICAL "CRITICAL"

// Shared logging function
// FIXME: seconds print
void log_message(const char *level, const char *process, const char *format,
				 ...) {
	FILE *logfile = fopen("simulation.log", "a");
	if (!logfile) {
		perror("Error opening log file");
		return;
	}

	const char date_format[] = "%Y-%m-%d %H:%M:%S";
	time_t now = time(NULL);
	char timestamp[sizeof(date_format)];
	strftime(timestamp, sizeof(timestamp), date_format, localtime(&now));

	va_list args;
	va_start(args, format);
	fprintf(logfile, "[%s] [%s] [%s] ", timestamp, process, level);
	vfprintf(logfile, format, args);
	fprintf(logfile, "\n");
	va_end(args);

	fclose(logfile);
}

#endif // LOGGING_H
