#ifndef TBOX_FLOW_EVENT_ACTION_H_20221105
#define TBOX_FLOW_EVENT_ACTION_H_20221105

#include "../action.h"
#include "../event_subscriber.h"
#include "../event_publisher.h"

namespace tbox {
namespace flow {

class EventAction : public Action,
                    public EventSubscriber {
  public:
    explicit EventAction(event::Loop &loop, EventPublisher &pub);

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;
    virtual void onFinished(bool succ) override;

  private:
    EventPublisher &pub_;
};

}
}

#endif //TBOX_FLOW_EVENT_ACTION_H_20221105
