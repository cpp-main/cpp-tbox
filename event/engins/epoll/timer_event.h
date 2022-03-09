#ifndef TBOX_EVENT_EPOLL_TIMER_EVENT_H_20200110
#define TBOX_EVENT_EPOLL_TIMER_EVENT_H_20200110

#include "tbox/base/cabinet.hpp"
#include "../../timer_event.h"

namespace tbox {
namespace event {

class EpollLoop;

class EpollTimerEvent : public TimerEvent {
  public:
    explicit EpollTimerEvent(EpollLoop *wp_loop);
    virtual ~EpollTimerEvent() override;

  public:
    virtual bool initialize(const std::chrono::milliseconds &interval, Mode mode) override;
    virtual void setCallback(const CallbackFunc &cb) override;

    virtual bool isEnabled() const override;
    virtual bool enable() override;
    virtual bool disable() override;

    virtual Loop* getLoop() const override;

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

    cabinet::Token token_;
};

}
}

#endif //TBOX_EVENT_EPOLL_TIMER_EVENT_H_20200110
