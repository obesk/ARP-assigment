#define PROCESS_NAME "BLACKBOARD"

#include "logging.h"
#include "pfds.h"

int main(int argc, char **argv) {
	/* char buffer[SERIALIZED_PFDS_SIZE]; */
	log_message(LOG_INFO, PROCESS_NAME, "Started blackboard with %d arguments",
				argc);

	PFDs *pfds = argsToPFDs(&argv[1]);
	log_message(LOG_DEBUG, PROCESS_NAME, "first fd: %d", pfds->read[0]);
}
