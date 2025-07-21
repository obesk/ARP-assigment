#include "logging.h"
#include "DDSMessagePubSubTypes.hpp"

#include <chrono>
#include <thread>
#include <unordered_map>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/rtps/transport/TCPv4TransportDescriptor.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>


using namespace eprosima::fastdds::dds;
using namespace eprosima::fastdds::rtps;

//TODO: move to .cpp

class BlackboardSubscriber {
private:
    DomainParticipant* participant_;
    Subscriber* subscriber_;
    DataReader* reader_;
    Topic* topic_;
    TypeSupport type_;

    class SubListener : public DataReaderListener {
    public:
        SubListener()
        { }

        ~SubListener() override { }

        void on_subscription_matched(
                DataReader*,
                const SubscriptionMatchedStatus& info) override {
            if (info.current_count_change == 1) {
                log_message(LOG_INFO, "subscriber matched") 
            } else if (info.current_count_change == -1) {
                log_message(LOG_INFO, "subscriber unmatched");
            } else {
                log_message(LOG_ERROR, "%d  is not a valid value for "
                    "SubscriptionMatchedStatus current count change",
                    info.current_count_change);
            }
        }

        void on_data_available(DataReader* reader) override {
            SampleInfo info;
            DDSMessage msg;
            if (reader->take_next_sample(&msg, &info) 
                == eprosima::fastdds::dds::RETCODE_OK) {
                if (info.valid_data) {
                    log_message(LOG_DEBUG, "received ok message");
                    message_order.push_back(msg.payload()._d());
                    message_queue[msg.payload()._d()] = msg;
                } else {
                    log_message(LOG_ERROR, "error while receiving message: "
                        "invalid data");
                }
            }
        }
        std::unordered_map<DDSMemorySector, DDSMessage> message_queue;
        std::deque<DDSMemorySector> message_order;
    }
    listener_;

public:

    BlackboardSubscriber()
        : participant_(nullptr)
        , subscriber_(nullptr)
        , topic_(nullptr)
        , reader_(nullptr)
        , type_(new DDSMessagePubSubType())
    { }

    virtual ~BlackboardSubscriber() {
        if (reader_ != nullptr) {
            subscriber_->delete_datareader(reader_);
        }
        if (topic_ != nullptr) {
            participant_->delete_topic(topic_);
        }
        if (subscriber_ != nullptr) {
            participant_->delete_subscriber(subscriber_);
        }
        DomainParticipantFactory::get_instance()->delete_participant(participant_);
    }

    //!Initialize the subscriber
    bool init(std::array<uint32_t, 4> server_ip, std::array<uint32_t, 4> client_ip, int server_port, int client_port) {
        DomainParticipantQos server_qos = PARTICIPANT_QOS_DEFAULT;

        // Set participant as SERVER
        server_qos.wire_protocol().builtin.discovery_config.discoveryProtocol =
                DiscoveryProtocol::SERVER;

        // Set SERVER's listening locator for PDP
        Locator_t locator;
        IPLocator::setIPv4(locator, server_ip[0], server_ip[1], server_ip[2], server_ip[3]);
        locator.port = server_port;
        server_qos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(locator);

        /* Add a remote serve to which this server will connect */
        // Set remote SERVER's listening locator for PDP
        Locator_t remote_locator;
        IPLocator::setIPv4(remote_locator, client_ip[0], client_ip[1], client_ip[2], client_ip[3]);
        remote_locator.port = client_port;

        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, server_qos);

        if (participant_ == nullptr) {
            return false;
        }

        // Register the Type
        type_.register_type(participant_);

        // Create the subscriptions Topic
        topic_ = participant_->create_topic("BlackboardTopic" +
             std::to_string(server_port), type_.get_type_name(),
             TOPIC_QOS_DEFAULT);

        if (topic_ == nullptr) {
            return false;
        }

        // Create the Subscriber
        subscriber_ = participant_->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);

        if (subscriber_ == nullptr) {
            return false;
        }

        // Create the DataReader
        reader_ = subscriber_->create_datareader(topic_, DATAREADER_QOS_DEFAULT, &listener_);

        if (reader_ == nullptr) {
            return false;
        }

        return true;
    }

    bool has_new_data() {
        return not listener_.message_queue.empty();
    }

    DDSMessage get_message() {
        while (!listener_.message_order.empty()) {
            const DDSMemorySector key = listener_.message_order.front();
            listener_.message_order.pop_front();
            const auto elem = listener_.message_queue.find(key);
            if(elem != listener_.message_queue.end()) {
                DDSMessage return_value = elem->second;
                listener_.message_queue.erase(elem->first);
                return return_value;
            }
        }
        return DDSMessage{};
    }

};