#ifndef TBOX_TIMER_CRON_TIMER_H
#define TBOX_TIMER_CRON_TIMER_H

#include <string>

#include <functional>
#include <tbox/event/forward.h>

namespace tbox {
namespace timer {
/*
 * @brief The linux crontab timer.
 *
 * CrontabTimer allow the user to make plans by linux crontab expression.
 *  Linux-cron tab expression
  *    *    *    *    *    *
  -    -    -    -    -    -
  |    |    |    |    |    |
  |    |    |    |    |    +----- day of week (0 - 7) (Sunday=0 or 7) OR sun,mon,tue,wed,thu,fri,sat
  |    |    |    |    +---------- month (1 - 12) OR jan,feb,mar,apr ...
  |    |    |    +--------------- day of month (1 - 31)
  |    |    +-------------------- hour (0 - 23)
  |    +------------------------- minute (0 - 59)
  +------------------------------ second (0 - 59)

 *
 * code example:
 *
 * Loop *loop = Loop::New();
 * CrontabTimer tmr(loop);
 * tmr.initialize("18 28 14 * * *", 480); // every day at 14:28:18 in +8 timezone
 * tmr.setCallback([] { std::cout << "timeout" << std::endl; });
 * tmr.enable();
 * loop->runLoop();
 */

class CrontabTimer
{
  public:
    CrontabTimer(event::Loop *wp_loop);
    virtual ~CrontabTimer();

    bool initialize(const std::string &crontab_expr, int timezone_offset_minutes);

    using Callback = std::function<void()>;
    void setCallback(const Callback &cb) { cb_ = cb; }

    bool isEnabled() const;
    bool enable();
    bool disable();

    /*
     * @brief refresh the internal timer.
     *
     * In order not to affect the accuracy of the timer,
     * you need to call this function to refresh the internal timer
     * when the system clock was changed.
     */
    void refresh();
    void cleanup();

  private:
    void onTimeExpired();
    bool setNextAlarm();

  private:
    event::Loop *wp_loop_;
    event::TimerEvent *sp_timer_ev_;
    void *sp_cron_expr_;
    int timezone_offset_seconds_ = 0;
    Callback cb_;
    int cb_level_ = 0;
};

}
}

#endif //TBOX_TIMER_CRON_TIMER_H
