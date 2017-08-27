#ifndef TBOX_EVENT_LIBEV_TIMER_EVENT_H_20170827
#define TBOX_EVENT_LIBEV_TIMER_EVENT_H_20170827

#include "../../timer_item.h"

#include <ev.h>

namespace tbox {
namespace event {

class LibevLoop;

class LibevTimerItem : public TimerItem {
  public:
    explicit LibevTimerItem(LibevLoop *wp_loop);
    virtual ~LibevTimerItem();

  public:
    virtual bool initialize(const Timespan &interval, Mode mode);
    virtual void setCallback(const CallbackFunc &cb);

    virtual bool isEnabled() const;
    virtual bool enable();
    virtual bool disable();

  protected:
    static void OnEventCallback(struct ev_loop*, ev_timer *p_w, int events);
    void onEvent();

  private:
    LibevLoop *wp_loop_;
    ev_timer timer_ev_;
    bool is_inited_;

    Timespan interval_;
    Mode mode_;

    CallbackFunc cb_;
    int cb_level_;
};

}
}

#endif //TBOX_EVENT_LIBEV_TIMER_EVENT_H_20170827
