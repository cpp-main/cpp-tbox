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
    virtual ~LibeventFdEvent() override;

  public:
    virtual bool initialize(int fd, short events, Mode mode) override;
    virtual void setCallback(const CallbackFunc &cb) override;

    virtual bool isEnabled() const override;
    virtual bool enable() override;
    virtual bool disable() override;

    virtual Loop* getLoop() const override;

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
