#include "blackboard_publisher_cif.h"
#include "blackboard_publisher.hpp"

extern "C" {
    BHandle blackboard_publisher_create() {
        return new BlackboardPublisher();
    }
    void blackboard_publisher_free(BHandle bp) {
        delete bp;
    }
    int blackboard_publisher_test(BHandle bp, int i) {
        return i;
    }
}