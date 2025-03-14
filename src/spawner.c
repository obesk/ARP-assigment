#include "processes.h"
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PROCESS_NAME "SPAWNER"
#define MAX_SIZE_PNAME = 50;

#include "logging.h"
#include "pfds.h"

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

	log_message(LOG_INFO, PROCESS_NAME, "spawner process starting");

	struct PFDs blackboard_pfds, processes_pfds;

	const bool result = newPFDs(&blackboard_pfds, &processes_pfds);

	if (!result) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Error while initializing struct PFDs");
		exit(1);
	}

	// it's better for the watchdog to be the first to spawn since it can
	// receive the signals immediately

	// spawning the watchdog
	const pid_t watchdog_pid = fork();

	if (watchdog_pid < 0) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Error while creating the watchdog process: %s",
					watchdog_executable);
		exit(1);
	}
	if (watchdog_pid == 0) {
		log_message(LOG_INFO, PROCESS_NAME,
					"Created watchdog child process with executable: %s",
					watchdog_executable);
		execl(watchdog_executable, watchdog_executable, NULL);
		return 0;
	}

	// spawning the blackboard
	char **args = allPFDsToArgs(&blackboard_pfds, blackboard_executable);

	const pid_t blackboard_pid = fork();
	if (blackboard_pid < 0) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Error while creating blackboard child process: %s",
					blackboard_executable);
		exit(1);
	}
	if (blackboard_pid == 0) {
		log_message(LOG_INFO, PROCESS_NAME,
					"Created blackboard child process with executable: %s",
					blackboard_executable);

		// closing all the unused pipe ends
		// FIXME: it may be a good idea to close unused pipes currently this
		// instruction cause problems, check before uncommenting
		// closeAllPFDs(&processes_pfds);
		execv(blackboard_executable, args);
		return 0;
	}

	// spawning processes
	for (int i = 0; i < PROCESS_N; ++i) {

		// (2 optional for the konsole spawn) + 1 for the program name + 2 for
		// the pfds +1 for the watchdog pid + 1 for the NULL required by execv
		const int n_args = 5 + (2 * spawn_in_konsole[i]);

		char **args = malloc((sizeof(char *) * n_args));

		// FIXME: by passing the already allocated pointer to PFDsToArgs it
		// should be possible to do the following operations in the function
		// or just manage the konsole option from it
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
		snprintf(args[args_count++], INT_STR_LEN, "%d", processes_pfds.read[i]);
		args[args_count] = malloc(INT_STR_LEN);
		snprintf(args[args_count++], INT_STR_LEN, "%d",
				 processes_pfds.write[i]);
		args[args_count] = malloc(INT_STR_LEN);
		snprintf(args[args_count++], INT_STR_LEN, "%d", watchdog_pid);

		args[args_count] = NULL;

		// char **args = PFDsToArgs(processes_pfds.read[i],
		// 						 processes_pfds.write[i], executables[i]);

		pid_t pid = fork();
		if (pid < 0) {
			log_message(LOG_CRITICAL, PROCESS_NAME,
						"Error while creating child process: %s",
						executables[i]);
			exit(1);
		}
		if (pid == 0) {
			log_message(LOG_INFO, PROCESS_NAME,
						"Created child process with executable: %s",
						executables[i]);

			// closing all unused pfds ends
			// FIXME: it may be a good idea to close unused pipes currently this
			// instruction cause problems, check before uncommenting
			// closeAllPFDs(&blackboard_pfds);
			// for (int j = 0; j < PROCESS_N; ++j) {
			// 	// keeping open only the pfds needed by the process
			// 	if (i == j) {
			// 		continue;
			// 	}
			// 	close(processes_pfds.read[j]);
			// 	close(processes_pfds.write[j]);
			// }
			execv(args[0], args);
		}
	}
}
