#ifndef TBOX_EVENT_LIBEV_TIMER_EVENT_H_20170827
#define TBOX_EVENT_LIBEV_TIMER_EVENT_H_20170827

#include "../../timer_event.h"

#include <ev.h>

namespace tbox {
namespace event {

class LibevLoop;

class LibevTimerEvent : public TimerEvent {
  public:
    explicit LibevTimerEvent(LibevLoop *wp_loop);
    virtual ~LibevTimerEvent() override;

  public:
    virtual bool initialize(const std::chrono::milliseconds &interval, Mode mode) override;
    virtual void setCallback(const CallbackFunc &cb) override;

    virtual bool isEnabled() const override;
    virtual bool enable() override;
    virtual bool disable() override;

    virtual Loop* getLoop() const override;

  protected:
    static void OnEventCallback(struct ev_loop*, ev_timer *p_w, int events);
    void onEvent();

  private:
    LibevLoop *wp_loop_;
    ev_timer timer_ev_;
    bool is_inited_;

    std::chrono::milliseconds interval_;
    Mode mode_;

    CallbackFunc cb_;
    int cb_level_;
};

}
}

#endif //TBOX_EVENT_LIBEV_TIMER_EVENT_H_20170827
