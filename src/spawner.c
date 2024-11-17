#define PROCESS_NAME "SPAWNER"

#include "logging.h"
#include "pfds.h"

int main(void) {
	log_message(LOG_INFO, PROCESS_NAME, "spawner process starting");

	PFDs *pfds = newPFDs();
	if (!pfds) {
		log_message(LOG_CRITICAL, PROCESS_NAME,
					"Error while initializing PFDs");
	}
	char **args = PFDsToArgs(pfds, "./bin/blackboard");
	log_message(LOG_DEBUG, PROCESS_NAME, "first fd: %d", pfds->read[0]);

	pid_t pid = fork();
	if (pid != 0) {
		return pid;
	} else {
		execv("./bin/blackboard", args);
	}
}
