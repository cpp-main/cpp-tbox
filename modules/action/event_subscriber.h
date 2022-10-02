#ifndef TBOX_ACTION_EVENT_SUBSCRIBER_H_20221001
#define TBOX_ACTION_EVENT_SUBSCRIBER_H_20221001

namespace tbox {
namespace action {

class EventSubscriber {
  public:
    virtual bool onEvent(int event_id, void *event_data) = 0;
};

}
}

#endif //TBOX_ACTION_EVENT_SUBSCRIBER_H_20221001
