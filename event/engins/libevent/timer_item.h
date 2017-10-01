#ifndef TBOX_EVENT_LIBEVENT_TIMER_ITEM_H_20170715
#define TBOX_EVENT_LIBEVENT_TIMER_ITEM_H_20170715

#include "../../timer_item.h"
#include <event2/event_struct.h>

namespace tbox {
namespace event {

class LibeventLoop;

class LibeventTimerItem : public TimerItem {
  public:
    explicit LibeventTimerItem(LibeventLoop *wp_loop);
    virtual ~LibeventTimerItem();

  public:
    virtual bool initialize(const Timespan &interval, Mode mode);
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
    Timespan interval_;
    CallbackFunc cb_;
    int cb_level_;
};

}
}

#endif //TBOX_EVENT_LIBEVENT_TIMER_ITEM_H_20170715
