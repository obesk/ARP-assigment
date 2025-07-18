#include "blackboard_publisher.h"

BlackboardPublisher::BlackboardPublisher()
	: participant_(nullptr)
	, publisher_(nullptr)
	, topic_(nullptr)
	, writer_(nullptr)
	, type_(new MessagePubSubType()) { }

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
	message_.index(0);
	message_.message("HelloWorld");

	DomainParticipantQos participantQos;
	participantQos.name("Participant_publisher");
	participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

	if (participant_ == nullptr) {
		return false;
	}

	// Register the Type
	type_.register_type(participant_);

	// Create the publications Topic
	topic_ = participant_->create_topic("HelloWorldTopic", "HelloWorld", TOPIC_QOS_DEFAULT);

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

bool BlackboardPublisher::publish() {
	if (listener_.matched_ > 0) {
		message_.index(message_.index() + 1);
		writer_->write(&message_);
		return true;
	}
	return false;
}

void BlackboardPublisher::run(uint32_t samples) {
	uint32_t samples_sent = 0;
	while (samples_sent < samples) {
		if (publish()) {
			samples_sent++;
			std::cout << "Message: " << message_.message() << " with index: " << message_.index()
						<< " SENT" << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

int hello() {
	return 1;
};
