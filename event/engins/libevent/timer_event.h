#ifndef TBOX_EVENT_LIBEVENT_TIMER_ITEM_H_20170715
#define TBOX_EVENT_LIBEVENT_TIMER_ITEM_H_20170715

#include "../../timer_event.h"
#include <event2/event_struct.h>

namespace tbox {
namespace event {

class LibeventLoop;

class LibeventTimerEvent : public TimerEvent {
  public:
    explicit LibeventTimerEvent(LibeventLoop *wp_loop);
    virtual ~LibeventTimerEvent();

  public:
    virtual bool initialize(const std::chrono::milliseconds &interval, Mode mode);
    virtual void setCallback(const CallbackFunc &cb);

    virtual bool isEnabled() const;
    virtual bool enable();
    virtual bool disable();

  protected:
    static void OnEventCallback(int, short, void *args);
    void onEvent();

  private:
    LibeventLoop *wp_loop_;
    struct event event_;
    bool is_inited_;
    struct timeval interval_;
    CallbackFunc cb_;
    int cb_level_;
};

}
}

#endif //TBOX_EVENT_LIBEVENT_TIMER_ITEM_H_20170715
