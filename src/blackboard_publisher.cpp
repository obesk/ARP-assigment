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

bool BlackboardPublisher::init() {
	DomainParticipantQos participantQos;
	participantQos.name("BlackboardPublisher");
	participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

	if (participant_ == nullptr) {
		return false;
	}

	// Register the Type
	type_.register_type(participant_);

	// Create the publications Topic
	topic_ = participant_->create_topic("BlackboardTopic", "DDSMessage", TOPIC_QOS_DEFAULT);
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

bool BlackboardPublisher::publish(DDSMessage message) {
	std::cerr << "publish" << std::endl;
	if (listener_.matched_ > 0) {
		std::cerr << "writing message" << std::endl;
		writer_->write(&message);
		std::cerr << "message wrote" << std::endl;
		return true;
	}
	return false;
}