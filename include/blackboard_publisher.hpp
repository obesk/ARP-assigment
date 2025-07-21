#ifndef BLACKBOARD_PUBLISHER_HPP
#define BLACKBOARD_PUBLISHER_HPP

#include "logging.h"

#include "DDSMessage.hpp"
#include "DDSMessagePubSubTypes.hpp"

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/DataWriterListener.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

#include <thread>

//TODO: better not to use this
using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

class BlackboardPublisher {
 private:
	DomainParticipant* participant_;
	Publisher* publisher_;
	Topic* topic_;
	DataWriter* writer_;
	TypeSupport type_;

	class PubListener : public DataWriterListener {
	 public:
		PubListener():
			matched_(0) {
		}
		~PubListener() override { }
		void on_publication_matched(DataWriter*, const PublicationMatchedStatus& info) override {
			// TODO: convert to log messages
			if (info.current_count_change == 1) {
				matched_ = info.total_count;
				log_message(LOG_INFO, "publisher matched");
			}
			else if (info.current_count_change == -1) {
				matched_ = info.total_count;
				log_message(LOG_INFO, "publisher unmatched");
			} else {
				log_message(LOG_ERROR, "%d is not a valid value for"
					"PublicationMatchedStatus current count change.",
					info.current_count_change);
			}
		}
		std::atomic_int matched_;
	} listener_;
 public:
	BlackboardPublisher();
	virtual ~BlackboardPublisher();

	bool init(std::array<uint32_t, 4> server_ip, int server_port);
	bool publish(DDSMessage message);
};

#endif //BLACKBOARD_PUBLISHER_HPP

