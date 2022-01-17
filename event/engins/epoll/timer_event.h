#ifndef TBOX_EVENT_EPOLL_TIMER_EVENT_H_20200110
#define TBOX_EVENT_EPOLL_TIMER_EVENT_H_20200110

#include "../../timer_event.h"


namespace tbox {
namespace event {

class EpollLoop;

class EpollTimerEvent : public TimerEvent {
  public:
    explicit EpollTimerEvent(EpollLoop *wp_loop);
    virtual ~EpollTimerEvent();

  public:
    virtual bool initialize(const std::chrono::milliseconds &interval, Mode mode);
    virtual void setCallback(const CallbackFunc &cb);

    virtual bool isEnabled() const;
    virtual bool enable();
    virtual bool disable();

    virtual Loop* getLoop() const;

  protected:
    void onEvent();

  private:
    EpollLoop *wp_loop_;
    bool is_inited_{ false };
    bool is_enabled_{ false };

    std::chrono::milliseconds interval_{ 0 };
    Mode mode_{ Mode::kOneshot };

    CallbackFunc cb_{ nullptr };
    int cb_level_{ 0 };

    uint64_t id_{ 0 };
};

}
}

#endif //TBOX_EVENT_EPOLL_TIMER_EVENT_H_20200110
