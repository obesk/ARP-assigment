#ifndef BLACKBOARD_PUBLISHER_H
#define BLACKBOARD_PUBLISHER_H

struct BlackboardPublisher;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BlackboardPublisher *BHandle;

BHandle blackboard_publisher_create();
void blackboard_publisher_free(BHandle bp);
int blackboard_publisher_test(BHandle bp, int i);

#ifdef __cplusplus
}
#endif

#endif //BLACKBOARD_PUBLISHER_H