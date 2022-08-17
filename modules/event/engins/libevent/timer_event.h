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
    virtual ~LibeventTimerEvent() override;

  public:
    virtual bool initialize(const std::chrono::milliseconds &interval, Mode mode) override;
    virtual void setCallback(const CallbackFunc &cb) override;

    virtual bool isEnabled() const override;
    virtual bool enable() override;
    virtual bool disable() override;

    virtual Loop* getLoop() const override;

  protected:
    static void OnEventCallback(int, short, void *args);
    void onEvent();

  private:
    LibeventLoop *wp_loop_;
    struct event event_;
    bool is_inited_ = false;
    struct timeval interval_ = { 0, 0 };
    CallbackFunc cb_;
    int cb_level_ = 0;
};

}
}

#endif //TBOX_EVENT_LIBEVENT_TIMER_ITEM_H_20170715
