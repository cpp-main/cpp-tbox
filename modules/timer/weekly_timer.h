#ifndef TBOX_TIMER_WEEKLY_TIMER_H_20221019
#define TBOX_TIMER_WEEKLY_TIMER_H_20221019

#include <string>

#include <functional>
#include <tbox/event/forward.h>

namespace tbox {
namespace timer {
/*
 * @brief The linux weekly timer.
 *
 * WeeklyTimer allow the user to make plans weekly
 *
 * code example:
 *
 * Loop *loop = Loop::New();
 * WeeklyTimer *tmr = new WeeklyTimer(loop);
 * tmr->initialize((8 * 60 * 60), "1111111", 0); // everyday at 08:00 in 0 timezone
 * tmr->setCallback([] { std::cout << "time is up" << endl;});
 * tmr->enable();
 * loop->runLoop();
 */

class WeeklyTimer
{
  public:
    explicit WeeklyTimer(event::Loop *wp_loop);
    virtual ~WeeklyTimer();

    bool initialize(int seconds_of_day, const std::string &week_mask, int timezone_offset_minutes = 0);

    using Callback = std::function<void()>;
    void setCallback(const Callback &cb) { cb_ = cb; }

    bool isEnabled() const;
    bool enable();
    bool disable();

    void cleanup();

  protected:
    int calculateWaitSeconds();
    bool activeTimer();

  private:
    event::Loop *wp_loop_;
    event::TimerEvent *sp_timer_ev_;

    int seconds_of_day_ = 0;
    std::string week_mask_;
    int timezone_offset_seconds_ = 0;

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

#endif //TBOX_TIMER_WEEKLY_TIMER_H_20221019
