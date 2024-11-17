#include "logging.h"

#define PROCESS_NAME "SPAWNER"

int main(void) {
	log_message(LOG_INFO, PROCESS_NAME, "spawner process starting");
	return 0;
}
