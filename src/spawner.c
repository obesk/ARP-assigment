#define PROCESS_NAME "SPAWNER"

#include "processes.h"
#include "logging.h"
#include "pfds.h"

#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_SIZE_PNAME = 50;

int main(void) {
	const char *const executables[PROCESS_N] = {
		[PROCESS_DRONE] = "./bin/drone",
		[PROCESS_INPUT] = "./bin/input",
		[PROCESS_MAP] = "./bin/map",
		[PROCESS_TARGETS] = "./bin/targets",
		[PROCESS_OBSTACLES] = "./bin/obstacles",
	};

	const bool spawn_in_konsole[PROCESS_N] = {
		[PROCESS_INPUT] = true,
		[PROCESS_MAP] = true,
	};

	const char *watchdog_executable = "./bin/watchdog";
	const char *blackboard_executable = "./bin/blackboard";

	log_message(LOG_INFO, "spawner process starting");

	struct PFDs blackboard_pfds, processes_pfds;

	const bool result = newPFDs(&blackboard_pfds, &processes_pfds);

	if (!result) {
		log_message(LOG_CRITICAL, "Error while initializing struct PFDs");
		exit(1);
	}

	// it's better for the watchdog to be the first to spawn since it can
	// receive the signals immediately

	// spawning the watchdog
	const pid_t watchdog_pid = fork();
	if (watchdog_pid < 0) {
		log_message(LOG_CRITICAL,
					"Error while creating the watchdog process: %s",
					watchdog_executable);
		exit(1);
	}
	if (watchdog_pid == 0) {
		log_message(LOG_INFO,
					"Created watchdog child process with executable: %s",
					watchdog_executable);
		closeAllPFDs(&processes_pfds);
		closeAllPFDs(&blackboard_pfds);
		execl(watchdog_executable, watchdog_executable, NULL);
		return 0;
	}

	// spawning the blackboard
	const pid_t blackboard_pid = fork();
	if (blackboard_pid < 0) {
		log_message(LOG_CRITICAL,
					"Error while creating blackboard child process: %s",
					blackboard_executable);
		exit(1);
	}
	if (blackboard_pid == 0) {
		log_message(LOG_INFO,
					"Created blackboard child process with executable: %s",
					blackboard_executable);

		// creating the args for the blackboard
		const int n_args_pfds = PROCESS_N * 2;
		// +1 for the watchdog, +1 for the NULL at the end
		const int n_args_blackboard = n_args_pfds + 2;
		int args_count = 0;
		char **args = malloc(sizeof(char *) * n_args_blackboard);
		args[args_count] = malloc(strlen(blackboard_executable) + 1);
		strcpy(args[args_count++], blackboard_executable);
		// pointer arithmetic is used to index both read and write pfds
		int *p = (int *)&blackboard_pfds;
		for (int i = 0; i < n_args_pfds; ++i) {
			args[args_count] = malloc(INT_STR_LEN);
			snprintf(args[args_count++], INT_STR_LEN, "%d", *(p++));
		}
		args[args_count] = malloc(INT_STR_LEN);
		snprintf(args[args_count++], INT_STR_LEN, "%d", watchdog_pid);
		args[args_count++] = NULL; // setting the last element to NULL for execv

		// since the malloc is done after the fork there is not need to free the
		// memory since it's reclaimed by the execev
		closeAllPFDs(&processes_pfds);
		execv(blackboard_executable, args);
		return 0;
	}

	// spawning the other processes
	for (int i = 0; i < PROCESS_N; ++i) {
		pid_t pid = fork();
		if (pid < 0) {
			log_message(LOG_CRITICAL, "Error while creating child process: %s",
						executables[i]);
			exit(1);
		}
		if (pid == 0) {
			log_message(LOG_INFO, "Created child process with executable: %s",
						executables[i]);
			// (2 optional for the konsole spawn) + 1 for the program name + 2
			// for the pfds +1 for the watchdog pid + 1 for the NULL required by
			// execv
			const int n_args = 5 + (2 * spawn_in_konsole[i]);

			char **args = malloc((sizeof(char *) * n_args));

			int args_count = 0;

			if (spawn_in_konsole[i]) {
				args[args_count++] = "/usr/bin/konsole";
				args[args_count++] = "-e";
			}

			args[args_count] = malloc(strlen(executables[i]) + 1);

			strcpy(args[args_count++], executables[i]);

			// converting the processes file descriptors and copying them on the
			// args
			args[args_count] = malloc(INT_STR_LEN);
			snprintf(args[args_count++], INT_STR_LEN, "%d",
					 processes_pfds.read[i]);
			args[args_count] = malloc(INT_STR_LEN);
			snprintf(args[args_count++], INT_STR_LEN, "%d",
					 processes_pfds.write[i]);
			args[args_count] = malloc(INT_STR_LEN);
			snprintf(args[args_count++], INT_STR_LEN, "%d", watchdog_pid);

			args[args_count++] = NULL;

			// closing all unused pfds ends
			closeAllPFDs(&blackboard_pfds);
			for (int j = 0; j < PROCESS_N; ++j) {
				// keeping open only the pfds needed by the process
				if (i == j) {
					continue;
				}
				close(processes_pfds.read[j]);
				close(processes_pfds.write[j]);
			}
			execv(args[0], args);
		}
	}
}
