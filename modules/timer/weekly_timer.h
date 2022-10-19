#ifndef TBOX_TIMER_WEEKLY_TIMER_H_20221019
#define TBOX_TIMER_WEEKLY_TIMER_H_20221019

#include <string>

#include <functional>
#include <tbox/event/forward.h>

namespace tbox {
namespace timer {
/*
 * @brief The linux crontab timer.
 *
 * WeeklyTimer allow the user to make plans by linux crontab expression.
 *
 * code example:
 *
 * Loop *loop = Loop::New();
 * WeeklyTimer *tm = new WeeklyTimer(loop);
 * tm->initialize("0111110", (8 * 60 * 60), 0); // every day at 08:00 at 0 timezone
 * tm->enable();
 * loop->start();
 */

class WeeklyTimer
{
  public:
    explicit WeeklyTimer(event::Loop *wp_loop);
    virtual ~WeeklyTimer();

    bool initialize(const std::string &week_mask, uint32_t seconds_of_day, int timezone_offset_minutes = 0);

    using Callback = std::function<void()>;
    void setCallback(const Callback &cb) { cb_ = cb; }

    bool isEnabled() const;
    bool enable();
    bool disable();

    void cleanup();

  private:
    event::Loop *wp_loop_;
    event::TimerEvent *sp_timer_ev_;
    Callback cb_;
};

}
}

#endif //TBOX_TIMER_WEEKLY_TIMER_H_20221019
