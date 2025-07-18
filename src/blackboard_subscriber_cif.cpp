#include "blackboard_subscriber_cif.h"
#include "blackboard_subscriber.hpp"

extern "C" {

#ifndef PROCESS_NAME
#define PROCESS_NAME "BLACKBOARD_SUBSCRIBER"
#endif

#include "logging.h"

BSubHandle blackboard_subscriber_create() {
	std::cerr << "blackboard subscriber creation" << std::endl;
	const BSubHandle handle =  new BlackboardSubscriber();
	std::cerr << "blackboard subscriber created" << std::endl;
	return handle;
}

bool blackboard_subscriber_init(BSubHandle bs) {
	std::cerr << "blackboard subscriber initialization" << std::endl;
	const bool ok = bs->init();
	std::cerr << "blackboard subscriber initialized" << std::endl;
	return ok;
}

void blackboard_subscriber_free(BSubHandle bs) {
	delete bs;
}

bool blackboard_subscriber_has_message(BSubHandle bs) {
	return bs->has_new_data();
}

struct Message blackboard_subscriber_get_message(BSubHandle bs) {
	const DDSMessage dds_msg = bs->get_message();
	struct Message msg {};
	msg.type = TYPE_SET;

	switch(dds_msg.payload()._d()) {
		case DDSMemorySector::DRONE_POSITION: {
			const DDSVec2D dds_drone_position = dds_msg.payload().drone_position();
			msg.sector = SECTOR_DRONE_POSITION; 
			msg.payload.drone_position = (struct Vec2D) { dds_drone_position.x(), dds_drone_position.y() };
		}
		 break;
		case DDSMemorySector::DRONE_FORCE: {
			const DDSVec2D dds_drone_force = dds_msg.payload().drone_force();
			msg.sector = SECTOR_DRONE_FORCE; 
			msg.payload.drone_force = (struct Vec2D) { dds_drone_force.x(), dds_drone_force.y() };
		}
		 break;
		case DDSMemorySector::DRONE_ACTUAL_FORCE: {
			const DDSVec2D dds_drone_actual_force = dds_msg.payload().drone_actual_force();
			msg.sector = SECTOR_DRONE_ACTUAL_FORCE; 
			msg.payload.drone_actual_force = (struct Vec2D) { dds_drone_actual_force.x(), dds_drone_actual_force.y() };
		}
		 break;

		//TODO: finish this
		// case DDSMemorySector::TARGETS:
		// case DDSMemorySector::OBSTACLES:
		default:
			return error_msg;
	}
	return msg;
}


} // etxtern "C"