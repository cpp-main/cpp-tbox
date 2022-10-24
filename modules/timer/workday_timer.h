#ifndef TBOX_TIMER_WORKDAY_TIMER_H_20221024
#define TBOX_TIMER_WORKDAY_TIMER_H_20221024

#include <string>

#include <functional>
#include <tbox/event/forward.h>

#include "timer.h"
#include "workday_calendar.h"

namespace tbox {
namespace timer {

class WorkdayTimer : public Timer
{
  public:
    using Timer::Timer;

    /**
     * \brief 初始化
     *
     * \param seconds_of_day  从本地00:00起到触发时间的秒数
     *                        如：08:30 = 8 * 3600 + 30 * 60 = 30600
     *
     * \param wp_calendar     TODO:
     */
    bool initialize(int seconds_of_day, WorkdayCalendar *wp_calendar, bool workday);

  protected:
    virtual int calculateWaitSeconds(uint32_t curr_local_ts) override;
    virtual bool onEnable();
    virtual bool onDisable();

  private:
    int seconds_of_day_ = 0;
    WorkdayCalendar *wp_calendar_ = nullptr;
    bool workday_ = true;
};

}
}

#endif //TBOX_TIMER_WORKDAY_TIMER_H_20221024
