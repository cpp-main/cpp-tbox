#ifndef TBOX_EVENTX_ACCURATE_TIMER_H_20230607
#define TBOX_EVENTX_ACCURATE_TIMER_H_20230607

#include <functional>
#include <chrono>

#include <tbox/event/fd_event.h>

namespace tbox {
namespace eventx {

class TimerFd {
  public:
    TimerFd(tbox::event::Loop *loop, const std::string &what);
    ~TimerFd();

  public:
    bool initialize(const std::chrono::nanoseconds &time_span, event::Event::Mode mode);

    using CallbackFunc = std::function<void ()>;
    void setCallback(CallbackFunc &&cb);

    bool isEnabled() const;
    bool enable();
    bool disable();

  private:
    void onEvent(short events);

  private:
    CallbackFunc cb_{ nullptr };
    tbox::event::Loop *loop_{ nullptr };
    tbox::event::FdEvent *timer_fd_event_{ nullptr };
    int timer_fd_{ -1 };
    bool is_inited_{ false };
    bool is_enabled_{ false };
    std::chrono::milliseconds interval_ { 0 };
    event::Event::Mode mode_{ event::Event::Mode::kOneshot };
};
}
}
#endif
