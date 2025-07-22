#include "blackboard_subscriber_cif.h"
#include "blackboard_subscriber.hpp"
#include "logging.h"

extern "C" {

#ifndef PROCESS_NAME
#define PROCESS_NAME "BLACKBOARD_SUBSCRIBER"
#endif

BSubHandle blackboard_subscriber_create() {
	const BSubHandle handle =  new BlackboardSubscriber();
	log_message(LOG_INFO, "blackboard subscriber created");
	return handle;
}

bool blackboard_subscriber_init(BSubHandle bs, int server_ip[4],
	int client_ip[4], int server_port, int client_port) {
	std::array<uint32_t, 4> arr_server_ip;
	std::array<uint32_t, 4> arr_client_ip;

	for (int i = 0; i < 4; ++i) {
		arr_server_ip[i] = server_ip[i];
		arr_client_ip[i] = client_ip[i];
	}


	const bool ok = bs->init(arr_server_ip, arr_client_ip,
		 server_port, client_port);
	log_message(LOG_INFO, "blackboard subscriber initialized");
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

	log_message(LOG_INFO, "received message with sector: %d", 
		(int)dds_msg.payload()._d());

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
		case DDSMemorySector::SCORE: {
			msg.payload.score = dds_msg.payload().score();
			msg.sector = SECTOR_SCORE; 
			std::cerr << "setting the score: " << msg.payload.score << std::endl;
		}
		 break;
		case DDSMemorySector::TARGETS: {
			const std::vector<DDSVec2D> dds_targets =
				dds_msg.payload().targets();
			msg.sector = SECTOR_TARGETS;
			for (int i = 0; i < dds_targets.size(); ++i) {
				msg.payload.targets.targets[i] = 
					{dds_targets[i].x(), dds_targets[i].y()};
			}
			msg.payload.targets.n = dds_targets.size();
		}
		 break;
		case DDSMemorySector::OBSTACLES: {
			const std::vector<DDSVec2D> dds_obstacles =
				dds_msg.payload().obstacles();
			msg.sector = SECTOR_OBSTACLES;
			for (int i = 0; i < dds_obstacles.size(); ++i) {
				msg.payload.obstacles.obstacles[i] = 
					{dds_obstacles[i].x(), dds_obstacles[i].y()};
			}
			msg.payload.obstacles.n = dds_obstacles.size();
		}
		 break;
		default:
			return error_msg;
	}
	return msg;
}


} // etxtern "C"