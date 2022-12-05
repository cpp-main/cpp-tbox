#ifndef TBOX_ACTION_EVENT_PUBLISHER_H_20221001
#define TBOX_ACTION_EVENT_PUBLISHER_H_20221001

#include "event.h"

namespace tbox {
namespace flow {

class EventSubscriber;

class EventPublisher {
  public:
    virtual void subscribe(EventSubscriber *subscriber) = 0;
    virtual void unsubscribe(EventSubscriber *subscriber) = 0;
    virtual void publish(Event event) = 0;

  protected:
    virtual ~EventPublisher() { }
};

}
}

#endif //TBOX_ACTION_EVENT_PUBLISHER_H_20221001
