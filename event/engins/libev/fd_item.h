#ifndef TBOX_EVENT_LIBEV_FD_EVENT_H_20170827
#define TBOX_EVENT_LIBEV_FD_EVENT_H_20170827

#include "../../fd_item.h"

#include <ev.h>

namespace tbox {
namespace event {

class LibevLoop;

class LibevFdItem : public FdItem {
  public:
    explicit LibevFdItem(LibevLoop *wp_loop);
    virtual ~LibevFdItem();

  public:
    virtual bool initialize(int fd, short events, Mode mode);
    virtual void setCallback(const CallbackFunc &cb);

    virtual bool isEnabled() const;
    virtual bool enable();
    virtual bool disable();

  protected:
    static void OnEventCallback(struct ev_loop *p_loop, ev_io *p_w, int events);
    void onEvent(short events);

  private:
    LibevLoop *wp_loop_;
    ev_io io_ev_;
    bool is_inited_;
    bool is_stop_after_trigger_;
    CallbackFunc cb_;
    int cb_level_;
};

}
}

#endif //TBOX_EVENT_LIBEV_FD_EVENT_H_20170827
