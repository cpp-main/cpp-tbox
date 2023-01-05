#ifndef TBOX_FLOW_EVENT_PUBLISHER_H_20221002
#define TBOX_FLOW_EVENT_PUBLISHER_H_20221002

#include <vector>
#include "event_publisher.h"

namespace tbox {
namespace flow {

class EventPublisherImpl : public EventPublisher {
  public:
    virtual void subscribe(EventSubscriber *subscriber) override;
    virtual void unsubscribe(EventSubscriber *subscriber) override;
    virtual void publish(Event event) override;

  private:
    std::vector<EventSubscriber*> subscriber_vec_;
    std::vector<EventSubscriber*> tmp_vec_;
};



}
}

#endif //TBOX_FLOW_EVENT_PUBLISHER_H_20221002
