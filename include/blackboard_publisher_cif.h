#ifndef BLACKBOARD_PUBLISHER_H
#define BLACKBOARD_PUBLISHER_H

struct BlackboardPublisher;

#ifdef __cplusplus
extern "C" {
#endif

#include "blackboard.h"

#include <stdbool.h>

typedef struct BlackboardPublisher *BPubHandle;

BPubHandle blackboard_publisher_create();
bool blackboard_publisher_init(BPubHandle bp, int ip[4], int port);
void blackboard_publisher_free(BPubHandle bp);
bool blackboard_queue_message(BPubHandle bp, const struct Message *message);
bool blackboard_try_publishing_messages(BPubHandle bp);

#ifdef __cplusplus
}
#endif

#endif //BLACKBOARD_PUBLISHER_H