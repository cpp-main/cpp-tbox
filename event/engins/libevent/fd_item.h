#ifndef TBOX_EVENT_LIBEVENT_FD_ITEM_H_20170714
#define TBOX_EVENT_LIBEVENT_FD_ITEM_H_20170714

#include "../../fd_item.h"
#include <event2/event_struct.h>

namespace tbox {
namespace event {

class LibeventLoop;

class LibeventFdItem : public FdItem {
  public:
    explicit LibeventFdItem(LibeventLoop *wp_loop);
    virtual ~LibeventFdItem();

  public:
    virtual bool initialize(int fd, short events, Mode mode);
    virtual void setCallback(const CallbackFunc &cb);

    virtual bool isEnabled() const;
    virtual bool enable();
    virtual bool disable();

  protected:
    static void OnEventCallback(int fd, short events, void *args);
    void onEvent(short events);

  private:
    LibeventLoop *wp_loop_;

    struct event event_;
    bool is_inited_;
    short events_;
    CallbackFunc cb_;
};

}
}

#endif //TBOX_EVENT_LIBEVENT_FD_ITEM_H_20170714
