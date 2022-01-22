#ifndef TBOX_EVENT_LIBEPOLL_FD_EVENT_H_20220110
#define TBOX_EVENT_LIBEPOLL_FD_EVENT_H_20220110

#include "../../fd_event.h"

#include <sys/epoll.h>

namespace tbox {
namespace event {

class BuiltinLoop;
struct EventData;

class EpollFdEvent : public FdEvent {
  public:
    explicit EpollFdEvent(BuiltinLoop *wp_loop);
    virtual ~EpollFdEvent();

  public:
    virtual bool initialize(int fd, short events, Mode mode);
    virtual void setCallback(const CallbackFunc &cb);

    virtual bool isEnabled() const;
    virtual bool enable();
    virtual bool disable();

    virtual Loop* getLoop() const;

  public:
    static void OnEventCallback(int fd, uint32_t events, void *obj);

  protected:
    void onEvent(short events);

  private:
    BuiltinLoop *wp_loop_;
    bool is_inited_{ false };
    bool is_enabled_{ false };
    bool is_stop_after_trigger_ { false };
    CallbackFunc cb_;

    int fd_ = -1;
    struct epoll_event ev_;

    int cb_level_{ 0 };

};

}
}

#endif //TBOX_EVENT_LIBEPOLL_FD_EVENT_H_20220110
