#ifndef TBOX_EVENT_LIBEVENT_LOOP_H_20170713
#define TBOX_EVENT_LIBEVENT_LOOP_H_20170713

#include "../../common_loop.h"

struct event_base;

namespace tbox {
namespace event {

class LibeventLoop : public CommonLoop {
  public:
    explicit LibeventLoop();
    virtual ~LibeventLoop();

  public:
    virtual void runLoop(Mode mode);
    virtual void exitLoop(const std::chrono::milliseconds &wait_time);

    virtual FdItem* newFdItem();
    virtual TimerItem* newTimerItem();
    virtual SignalItem* newSignalItem();

  public:
    struct event_base* getEventBasePtr() const { return sp_event_base_; }

  private:
    struct event_base* sp_event_base_;
};

}
}

#endif //TBOX_EVENT_LIBEVENT_LOOP_H_20170713
