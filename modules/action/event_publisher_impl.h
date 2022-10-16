#ifndef TBOX_ACTION_EVENT_PUBLISHER_H_20221002
#define TBOX_ACTION_EVENT_PUBLISHER_H_20221002

#include <vector>
#include "event_publisher.h"
#include "event_subscriber.h"

namespace tbox {
namespace action {

class EventSubscriber;

class EventPublisherImpl : public EventPublisher {
  public:
    virtual void subscribe(EventSubscriber *subscriber) override;
    virtual void unsubscribe(EventSubscriber *subscriber) override;

  public:
    virtual void onEvent(Event event) override;

  private:
    std::vector<EventSubscriber*> subscriber_vec_;
};



}
}

#endif //TBOX_ACTION_EVENT_PUBLISHER_H_20221002
