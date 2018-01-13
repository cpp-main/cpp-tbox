#ifndef TBOX_EVENT_LIBEVENT_FD_ITEM_H_20170714
#define TBOX_EVENT_LIBEVENT_FD_ITEM_H_20170714

#include "../../fd_event.h"
#include <event2/event_struct.h>

namespace tbox {
namespace event {

class LibeventLoop;

class LibeventFdEvent : public FdEvent {
  public:
    explicit LibeventFdEvent(LibeventLoop *wp_loop);
    virtual ~LibeventFdEvent();

  public:
    virtual bool initialize(int fd, short events, Mode mode);
    virtual void setCallback(const CallbackFunc &cb);

    virtual bool isEnabled() const;
    virtual bool enable();
    virtual bool disable();

    virtual Loop* getLoop() const;

  protected:
    static void OnEventCallback(int fd, short events, void *args);
    void onEvent(short events);

  private:
    LibeventLoop *wp_loop_;

    struct event event_;
    bool is_inited_;
    short events_;
    CallbackFunc cb_;
    int cb_level_;
};

}
}

#endif //TBOX_EVENT_LIBEVENT_FD_ITEM_H_20170714
