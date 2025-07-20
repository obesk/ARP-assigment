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
	//std::cerr << "blackboard publisher initialization" << std::endl;
	const bool ok = bp->init({127, 0, 0, 1}, 11812);
	//std::cerr << "blackboard publisher initialized" << std::endl;
	return ok;
}

void blackboard_publisher_free(BPubHandle bp) {
	delete bp;
}

bool blackboard_publish_message(BPubHandle bp, const struct Message *message) {
	//std::cerr << "composing message" << std::endl;
	DDSPayload payload {};
	switch (message->sector) {
		//TODO: not all sectors managed
		case SECTOR_DRONE_POSITION: {
			//std::cerr << "drone position" << std::endl;
			const struct Vec2D drone_position = message->payload.drone_position;
			DDSVec2D dds_drone_position {};
			dds_drone_position.x(drone_position.x);
			dds_drone_position.y(drone_position.y);
			payload.drone_position(dds_drone_position);
		 }
		 break;
		case SECTOR_DRONE_FORCE: {
			//std::cerr << "drone force" << std::endl;
			const struct Vec2D drone_force = message->payload.drone_force;
			DDSVec2D dds_drone_force {};
			dds_drone_force.x(drone_force.x);
			dds_drone_force.y(drone_force.y);
			payload.drone_force(dds_drone_force);
		 }
		 break;
		case SECTOR_DRONE_ACTUAL_FORCE: {
			//std::cerr << "drone actual force" << std::endl;
			const struct Vec2D drone_actual_force = 
				message->payload.drone_actual_force;
			DDSVec2D dds_drone_actual_force {};
			dds_drone_actual_force.x(drone_actual_force.x);
			dds_drone_actual_force.y(drone_actual_force.y);
			//std::cerr << "actual force initialized" << std::endl;
			payload.drone_actual_force(dds_drone_actual_force);
			//std::cerr << "payload initialized" << std::endl;
		 }
		 break;
		case SECTOR_TARGETS: {
			const struct Targets targets = message->payload.targets;
			std::vector<DDSVec2D> dds_targets;
			dds_targets.reserve(targets.n);
			for (int i = 0; i < targets.n; ++i) {
				DDSVec2D target {};
				target.x(targets.targets[i].x);
				target.y(targets.targets[i].y);
				dds_targets.push_back(target);
			}
			//std::cerr << "targets published" <<std::endl;
		 }
		 break;
		case SECTOR_OBSTACLES: {
			const struct Obstacles obstacles = message->payload.obstacles;
			std::vector<DDSVec2D> dds_obstacles;
			dds_obstacles.reserve(obstacles.n);
			for (int i = 0; i < obstacles.n; ++i) {
				DDSVec2D obstacle {};
				obstacle.x(obstacles.obstacles[i].x);
				obstacle.y(obstacles.obstacles[i].y);
				dds_obstacles.push_back(obstacle);
			}
			//std::cerr << "obstacles published" <<std::endl;
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