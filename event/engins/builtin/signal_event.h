#ifndef TBOX_EVENT_EPOLL_SINGAL_EVENT_H_20220110
#define TBOX_EVENT_EPOLL_SINGAL_EVENT_H_20220110

#include "../../signal_event.h"

struct epoll_event;

namespace tbox {
namespace event {

class BuiltinLoop;
class EpollFdEvent;

class EpollSignalEvent : public SignalEvent {
  public:
    explicit EpollSignalEvent(BuiltinLoop *wp_loop);
    virtual ~EpollSignalEvent();

  public:
    virtual bool initialize(int signum, Mode mode);
    virtual void setCallback(const CallbackFunc &cb);

    virtual bool isEnabled() const;
    virtual bool enable();
    virtual bool disable();

    virtual Loop* getLoop() const;

  protected:
    void onEvent(short events);

  private:
    BuiltinLoop *wp_loop_{ nullptr };
    EpollFdEvent *signal_fd_event_{ nullptr };

    bool is_inited_{ false };
    bool is_stop_after_trigger_{ false };
    CallbackFunc cb_{ nullptr };
    int cb_level_{ 0 };
    int signal_fd_{ -1 };
};

}
}

#endif //TBOX_EVENT_EPOLL_SINGAL_EVENT_H_20220110
