#include <string.h>
#include <unistd.h>

#define PROCESS_NAME "SPAWNER"
#define MAX_SIZE_PNAME = 50;

#include "logging.h"
#include "pfds.h"

int main(void) {

	const char *const executables[N_PROCESSES] = {
		[DRONE] = "./bin/drone",
	};

	const char *blackboard_executable = "./bin/blackboard";

	log_message(LOG_INFO, PROCESS_NAME, "spawner process starting");

	PFDs blackboard_pfds, processes_pfds;

	const bool result = newPFDs(&blackboard_pfds, &processes_pfds);

	if (!result) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Error while initializing PFDs");
		exit(1);
	}

	// spawning the blackboard
	char **args = allPFDsToArgs(&blackboard_pfds, blackboard_executable);

	pid_t pid = fork();
	if (pid < 0) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Error while creating blackboard child process: %s",
					blackboard_executable);
		exit(1);
	}
	if (pid == 0) {
		log_message(LOG_INFO, PROCESS_NAME,
					"Created blackboard child process with executable: %s",
					blackboard_executable);

		// closing all the unused pipe ends
		closeAllPFDs(&processes_pfds);
		execv(blackboard_executable, args);
	}

	for (int i = 0; i < N_PROCESSES; ++i) {
		char **args = PFDsToArgs(processes_pfds.read[i],
								 processes_pfds.write[i], executables[i]);

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
			closeAllPFDs(&blackboard_pfds);
			for (int j = 0; j < N_PROCESSES; ++j) {
				// keeping open only the pfds needed by the process
				if (i == j) {
					continue;
				}
				close(processes_pfds.read[j]);
				close(processes_pfds.write[j]);
			}
			execv(executables[i], args);
		}
	}
}
