#ifndef TBOX_FLOW_EVENT_SUBSCRIBER_H_20221001
#define TBOX_FLOW_EVENT_SUBSCRIBER_H_20221001

#include "event.h"

namespace tbox {
namespace flow {

class EventSubscriber {
  public:
    virtual bool onEvent(Event event) = 0;

  protected:
    virtual ~EventSubscriber() { }
};

}
}

#endif //TBOX_FLOW_EVENT_SUBSCRIBER_H_20221001
