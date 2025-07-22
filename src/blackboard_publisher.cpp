#include "blackboard_publisher.hpp"

#include "DDSMessage.hpp"
#include "DDSMessagePubSubTypes.hpp"

BlackboardPublisher::BlackboardPublisher()
	: participant_(nullptr)
	, publisher_(nullptr)
	, topic_(nullptr)
	, writer_(nullptr)
	, type_(new DDSMessagePubSubType()) { }

BlackboardPublisher::~BlackboardPublisher() {
	if (writer_ != nullptr) {
		publisher_->delete_datawriter(writer_);
	}

	if (publisher_ != nullptr) {
		participant_->delete_publisher(publisher_);
	}

	if (topic_ != nullptr) {
		participant_->delete_topic(topic_);
	}

	DomainParticipantFactory::get_instance()->delete_participant(participant_);
}

bool BlackboardPublisher::init(std::array<uint32_t, 4> server_ip, int server_port) {
	DomainParticipantQos client_qos = PARTICIPANT_QOS_DEFAULT;

    client_qos.wire_protocol().builtin.discovery_config.discoveryProtocol =
             DiscoveryProtocol::CLIENT;

     // Set SERVER's listening locator for PDP
    Locator_t locator;
    IPLocator::setIPv4(locator, server_ip[0], server_ip[1], server_ip[2], server_ip[3]);
    locator.port = server_port;

     client_qos.wire_protocol().builtin.discovery_config.m_DiscoveryServers.push_back(locator);

     // Set ping period to 250 ms
     client_qos.wire_protocol().builtin.discovery_config.discoveryServer_client_syncperiod =
         Duration_t(0, 250000000);

	participant_ = DomainParticipantFactory::get_instance()->create_participant(0, client_qos);

	if (participant_ == nullptr) {
		return false;
	}

	// Register the Type
	type_.register_type(participant_);

	// Create the publications Topic
	topic_ = participant_->create_topic("BlackboardTopic" +
		std::to_string(server_port), type_.get_type_name(), TOPIC_QOS_DEFAULT);
	if (topic_ == nullptr) {
		return false;
	}

	// Create the Publisher
	publisher_ = participant_->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
	if (publisher_ == nullptr) {
		return false;
	}

	// Create the DataWriter
	writer_ = publisher_->create_datawriter(topic_, DATAWRITER_QOS_DEFAULT, &listener_);
	if (writer_ == nullptr) {
		return false;
	}
	return true;
}

bool BlackboardPublisher::queue_message(DDSMessage message) {
	message_queue[message.payload()._d()] = message;
	return true;
}

bool BlackboardPublisher::try_publishing_messages() {
	if (listener_.matched_ <= 0) {
		return false;
	}

	for (auto it = message_queue.cbegin(); it != message_queue.cend(); ) {
		if (writer_->write(&it->second) == RETCODE_OK) {
			message_queue.erase(it++);
		} else {
			++it;
		}
	}

	return message_queue.empty();
}