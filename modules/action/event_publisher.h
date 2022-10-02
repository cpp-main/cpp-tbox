#ifndef TBOX_ACTION_EVENT_PUBLISHER_H_20221001
#define TBOX_ACTION_EVENT_PUBLISHER_H_20221001

namespace tbox {
namespace action {

class EventSubscriber;

class EventPublisher {
  public:
    virtual ~EventPublisher() { }

  public:
    virtual void subscribe(EventSubscriber *subscriber) = 0;
    virtual void unsubscribe(EventSubscriber *subscriber) = 0;

  public:
    virtual void onEvent(int event_id, void *event_data) = 0;
};

}
}

#endif //TBOX_ACTION_EVENT_PUBLISHER_H_20221001
