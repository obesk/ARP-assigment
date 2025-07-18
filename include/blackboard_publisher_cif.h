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
bool blackboard_publisher_init(BPubHandle bp);
void blackboard_publisher_free(BPubHandle bp);
bool blackboard_publish_message(BPubHandle bp, const struct Message *message);

#ifdef __cplusplus
}
#endif

#endif //BLACKBOARD_PUBLISHER_H