#include "DDSMessagePubSubTypes.hpp"

#include <chrono>
#include <thread>

#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/DataReaderListener.hpp>
#include <fastdds/dds/subscriber/qos/DataReaderQos.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>

using namespace eprosima::fastdds::dds;

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
                std::cerr << "Subscriber matched." << std::endl;
            } else if (info.current_count_change == -1) {
                std::cerr << "Subscriber unmatched." << std::endl;
            } else {
                std::cerr << info.current_count_change
                          << " is not a valid value for SubscriptionMatchedStatus current count change" << std::endl;
            }
        }

        void on_data_available(DataReader* reader) override {
            SampleInfo info;
            if (reader->take_next_sample(&message_, &info) == eprosima::fastdds::dds::RETCODE_OK) {
                if (info.valid_data) {
                    new_data_ = true;
                    std::cerr << "Message with _d: " << (int) message_.payload()._d() << std::endl;
                }
            }
        }
        DDSMessage message_;
        bool new_data_;
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
    bool init() {
        DomainParticipantQos participantQos;
        participantQos.name("Participant_subscriber");
        participant_ = DomainParticipantFactory::get_instance()->create_participant(0, participantQos);

        if (participant_ == nullptr) {
            return false;
        }

        // Register the Type
        type_.register_type(participant_);

        // Create the subscriptions Topic
        topic_ = participant_->create_topic("BlackboardTopic", "DDSMessage", TOPIC_QOS_DEFAULT);

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
        return listener_.new_data_;
    }

    DDSMessage get_message() {
        return listener_.message_;
    }

};