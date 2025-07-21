#include "blackboard_publisher_cif.h"
#include "blackboard_publisher.hpp"

extern "C" {

#ifndef PROCESS_NAME
#define PROCESS_NAME "BLACKBOARD_PUBLISHER"
#endif

#include "logging.h"

BPubHandle blackboard_publisher_create() {
	const BPubHandle handle =  new BlackboardPublisher();
	log_message(LOG_INFO, "blackboard publisher created");
	return handle;
}

bool blackboard_publisher_init(BPubHandle bp, int ip[4], int port) {
	std::array<uint32_t, 4> server_ip;

	for (int i = 0; i < 4; ++i) {
		server_ip[i] = ip[i];
	}

	const bool ok = bp->init(server_ip, port);
	log_message(LOG_INFO, "blackboard publisher initialized");
	return ok;
}

void blackboard_publisher_free(BPubHandle bp) {
	delete bp;
}

bool blackboard_publish_message(BPubHandle bp, const struct Message *message) {
	DDSPayload payload {};
	std::string thing;
	switch (message->sector) {
		case SECTOR_DRONE_POSITION: {
			const struct Vec2D drone_position = message->payload.drone_position;
			DDSVec2D dds_drone_position {};
			dds_drone_position.x(drone_position.x);
			dds_drone_position.y(drone_position.y);
			payload.drone_position(dds_drone_position);
		 }
		 break;
		case SECTOR_DRONE_FORCE: {
			const struct Vec2D drone_force = message->payload.drone_force;
			DDSVec2D dds_drone_force {};
			dds_drone_force.x(drone_force.x);
			dds_drone_force.y(drone_force.y);
			payload.drone_force(dds_drone_force);
		 }
		 break;
		case SECTOR_DRONE_ACTUAL_FORCE: {
			const struct Vec2D drone_actual_force = 
				message->payload.drone_actual_force;
			DDSVec2D dds_drone_actual_force {};
			dds_drone_actual_force.x(drone_actual_force.x);
			dds_drone_actual_force.y(drone_actual_force.y);
			payload.drone_actual_force(dds_drone_actual_force);
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
			payload.targets(dds_targets);
			thing = "targts";
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
			payload.obstacles(dds_obstacles);
			thing = "obstacles";
		 }
		 break;
		default:
			return false;
	} 
	DDSMessage dds_message {};
	dds_message.payload(payload);
	bool ok = bp->publish(dds_message);
	log_message(LOG_INFO, "%s published", thing.c_str());
	return ok;

}


} // etxtern "C"