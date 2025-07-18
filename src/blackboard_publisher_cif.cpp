#include "blackboard_publisher_cif.h"
#include "blackboard_publisher.hpp"

extern "C" {

#ifndef PROCESS_NAME
#define PROCESS_NAME "BLACKBOARD_PUBLISHER"
#endif

#include "logging.h"

BPubHandle blackboard_publisher_create() {
	std::cerr << "blackboard publisher creation" << std::endl;
	const BPubHandle handle =  new BlackboardPublisher();
	std::cerr << "blackboard publisher created" << std::endl;
	return handle;
}

bool blackboard_publisher_init(BPubHandle bp) {
	std::cerr << "blackboard publisher initialization" << std::endl;
	const bool ok = bp->init();
	std::cerr << "blackboard publisher initialized" << std::endl;
	return ok;
}

void blackboard_publisher_free(BPubHandle bp) {
	delete bp;
}

bool blackboard_publish_message(BPubHandle bp, const struct Message *message) {
	std::cerr << "composing message" << std::endl;
	DDSPayload payload {};
	switch (message->sector) {
		//TODO: not all sectors managed
		case SECTOR_DRONE_POSITION: {
			std::cerr << "drone position" << std::endl;
			const struct Vec2D drone_position = message->payload.drone_position;
			DDSVec2D dds_drone_position {};
			dds_drone_position.x(drone_position.x);
			dds_drone_position.y(drone_position.y);
			payload.drone_position(dds_drone_position);
		 }
		 break;
		case SECTOR_DRONE_FORCE: {
			std::cerr << "drone force" << std::endl;
			const struct Vec2D drone_force = message->payload.drone_force;
			DDSVec2D dds_drone_force {};
			dds_drone_force.x(drone_force.x);
			dds_drone_force.y(drone_force.y);
			payload.drone_force(dds_drone_force);
		 }
		 break;
		case SECTOR_DRONE_ACTUAL_FORCE: {
			std::cerr << "drone actual force" << std::endl;
			const struct Vec2D drone_actual_force = 
				message->payload.drone_actual_force;
			DDSVec2D dds_drone_actual_force {};
			dds_drone_actual_force.x(drone_actual_force.x);
			dds_drone_actual_force.y(drone_actual_force.y);
			std::cerr << "actual force initialized" << std::endl;
			payload.drone_actual_force(dds_drone_actual_force);
			std::cerr << "payload initialized" << std::endl;
		 }
		 break;
		default:
			return false;
	} 
	DDSMessage dds_message {};
	dds_message.payload(payload);
	return bp->publish(dds_message);
}


} // etxtern "C"