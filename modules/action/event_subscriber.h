#ifndef TBOX_ACTION_EVENT_SUBSCRIBER_H_20221001
#define TBOX_ACTION_EVENT_SUBSCRIBER_H_20221001

#include "event.h"

namespace tbox {
namespace action {

class EventSubscriber {
  public:
    virtual bool onEvent(Event event) = 0;
};

}
}

#endif //TBOX_ACTION_EVENT_SUBSCRIBER_H_20221001
