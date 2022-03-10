#ifndef TBOX_EVENT_LIBEV_FD_EVENT_H_20170827
#define TBOX_EVENT_LIBEV_FD_EVENT_H_20170827

#include "../../fd_event.h"

#include <ev.h>

namespace tbox {
namespace event {

class LibevLoop;

class LibevFdEvent : public FdEvent {
  public:
    explicit LibevFdEvent(LibevLoop *wp_loop);
    virtual ~LibevFdEvent();

  public:
    virtual bool initialize(int fd, short events, Mode mode);
    virtual void setCallback(const CallbackFunc &cb);

    virtual bool isEnabled() const;
    virtual bool enable();
    virtual bool disable();

    virtual Loop* getLoop() const;

  protected:
    static void OnEventCallback(struct ev_loop *p_loop, ev_io *p_w, int events);
    void onEvent(short events);

  private:
    LibevLoop *wp_loop_;
    ev_io io_ev_;
    bool is_inited_ = false;
    bool is_stop_after_trigger_ = false;
    CallbackFunc cb_;
    int cb_level_ = 0;
};

}
}

#endif //TBOX_EVENT_LIBEV_FD_EVENT_H_20170827
