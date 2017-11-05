#ifndef TBOX_EVENT_LIBEV_SINGAL_EVENT_H_20170827
#define TBOX_EVENT_LIBEV_SINGAL_EVENT_H_20170827

#include "../../signal_event.h"

#include <ev.h>

namespace tbox {
namespace event {

class LibevLoop;

class LibevSignalEvent : public SignalEvent {
  public:
    explicit LibevSignalEvent(LibevLoop *wp_loop);
    virtual ~LibevSignalEvent();

  public:
    virtual bool initialize(int signum, Mode mode);
    virtual void setCallback(const CallbackFunc &cb);

    virtual bool isEnabled() const;
    virtual bool enable();
    virtual bool disable();

  protected:
    static void OnEventCallback(struct ev_loop *p_loop, ev_signal *p_w, int events);
    void onEvent();

  private:
    LibevLoop *wp_loop_;
    ev_signal signal_ev_;
    bool is_inited_;
    bool is_stop_after_trigger_;
    CallbackFunc cb_;
    int cb_level_;
};

}
}

#endif //TBOX_EVENT_LIBEV_SINGAL_EVENT_H_20170827
