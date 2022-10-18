#ifndef TBOX_CRON_TIMER_H
#define TBOX_CRON_TIMER_H

#include <string>
#include <functional>
#include "tbox/event/event.h"
#include "tbox/event/forward.h"

namespace tbox {
namespace event {
/*
 * @brief The linux crontab timer.
 *
 * CrontabTimer allow the user to make plains by linux crontab expression.
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
 * CrontabTimer *tm = new CrontabTimer(loop);
 * tm->initialize("18 28 14 * * *", [] { std::cout << "timeout" << std::endl; }); // every day at 14:28:18
 * tm->enable();
 * loop->start();
 */

class CrontabTimer : public Event
{
  public:
    CrontabTimer(Loop *wp_loop);
    virtual ~CrontabTimer();

    using Callback = std::function<void()>;
    bool initialize(const std::string &crontab_str, Callback cb);

    virtual bool isEnabled() const override;
    virtual bool enable() override;
    virtual bool disable() override;

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
    Loop *wp_loop_;
    TimerEvent *sp_timer_ev_;
    Callback cb_;
    void *sp_cron_expr_;
};

}
}

#endif
