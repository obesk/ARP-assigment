#ifndef BLACKBOARD_SUBSCRIBER_H
#define BLACKBOARD_SUBSCRIBER_H

struct BlackboardSubscriber;

#ifdef __cplusplus
extern "C" {
#endif

#include "blackboard.h"

#include <stdbool.h>

typedef struct BlackboardSubscriber *BSubHandle;

BSubHandle blackboard_subscriber_create();
bool blackboard_subscriber_init(BSubHandle bs);
void blackboard_subscriber_free(BSubHandle bs);


bool blackboard_subscriber_has_message(BSubHandle bs);
struct Message blackboard_subscriber_get_message(BSubHandle bs);

#ifdef __cplusplus
}
#endif

#endif //BLACKBOARD_SUBSCRIBER_H