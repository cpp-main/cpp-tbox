#include "workday_calendar.h"
#include <algorithm>
#include "workday_timer.h"

namespace tbox {
namespace timer {

void WorkdayCalendar::updateSpecialDays(const std::map<int, bool> &special_days) {
  special_days_ = special_days;
  for (auto timer : watch_timers_)
    timer->refresh();
}

void WorkdayCalendar::updateWeekMask(uint8_t week_mask) {
  week_mask_ = week_mask;
  for (auto timer : watch_timers_)
    timer->refresh();
}

void WorkdayCalendar::subscribe(WorkdayTimer *timer) {
  watch_timers_.push_back(timer);
}

void WorkdayCalendar::unsubscribe(WorkdayTimer *timer) {
  auto iter = std::remove(watch_timers_.begin(), watch_timers_.end(), timer);
  watch_timers_.erase(iter, watch_timers_.end());
}

bool WorkdayCalendar::isWorkay(int day_index) const {
  auto iter = special_days_.find(day_index);
  if (iter != special_days_.end()) {
    return iter->second;
  } else {
    int curr_week = ((day_index % 7) + 4) % 7;
    return (week_mask_ & (1 << curr_week));
  }
}

}
}
