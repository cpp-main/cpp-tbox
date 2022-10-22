#ifndef TBOX_TIMER_TIMER_H_20221022
#define TBOX_TIMER_TIMER_H_20221022

#include <string>

#include <functional>
#include <tbox/event/forward.h>

namespace tbox {
namespace timer {

class Timer
{
  public:
    explicit Timer(event::Loop *wp_loop);
    virtual ~Timer();

    using Callback = std::function<void()>;
    void setCallback(const Callback &cb) { cb_ = cb; }
    void setTimezone(int offset_minutes);

    bool isEnabled() const;
    bool enable();
    bool disable();

    void cleanup();

  protected:
    virtual int calculateWaitSeconds(uint32_t curr_local_ts) = 0;
    bool activeTimer();

    virtual bool onEnable() { return true; }
    virtual bool onDisable() { return true; }
    virtual void onCleanup() {}

  protected:
    event::Loop *wp_loop_;
    event::TimerEvent *sp_timer_ev_;

    bool using_independ_timezone_ = false;
    int timezone_offset_seconds_ = 0;

    int cb_level_ = 0;
    Callback cb_;

    enum class State {
      kNone,
      kInited,
      kRunning
    };
    State state_ = State::kNone;
};

}
}

#endif //TBOX_TIMER_TIMER_H_20221022
