#ifndef TBOX_TIMER_WORKDAY_CALENDAR_H_20221024
#define TBOX_TIMER_WORKDAY_CALENDAR_H_20221024

#include <map>
#include <vector>

namespace tbox {
namespace timer {

class WorkdayTimer;

class WorkdayCalendar {
  public:
    void updateSpecialDays(const std::map<int, bool> &special_days);
    void updateWeekMask(uint8_t week_mask);

    void subscribe(WorkdayTimer *timer);
    void unsubscribe(WorkdayTimer *timer);
    bool isWorkay(int day_index) const;

  private:
    uint8_t week_mask_ = 0x3e;
    std::map<int, bool> special_days_;
    std::vector<WorkdayTimer*> watch_timers_;
};

}
}

#endif //TBOX_TIMER_WORKDAY_CALENDAR_H_20221024
