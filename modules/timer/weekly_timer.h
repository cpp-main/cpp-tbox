#ifndef TBOX_TIMER_WEEKLY_TIMER_H_20221019
#define TBOX_TIMER_WEEKLY_TIMER_H_20221019

#include <string>

#include <functional>
#include <tbox/event/forward.h>

#include "timer.h"

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
 *
 * WeeklyTimer tmr(loop);
 * tmr.initialize((8 * 60 * 60), "1111111"); // everyday at 08:00
 * tmr.setCallback([] { std::cout << "time is up" << endl;});
 * tmr.enable();
 *
 * loop->runLoop();
 */

class WeeklyTimer : public Timer
{
  public:
    using Timer::Timer;

    bool initialize(int seconds_of_day, const std::string &week_mask);

  protected:
    virtual int calculateWaitSeconds(uint32_t curr_local_ts) override;

  private:
    int seconds_of_day_ = 0;
    std::string week_mask_;
};

}
}

#endif //TBOX_TIMER_WEEKLY_TIMER_H_20221019
