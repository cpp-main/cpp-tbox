#include "workday_alarm.h"

#include <sys/time.h>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>

#include "workday_calendar.h"

namespace tbox {
namespace alarm {

namespace {
constexpr auto kSecondsOfDay = 60 * 60 * 24;
}

bool WorkdayAlarm::initialize(int seconds_of_day, WorkdayCalendar *wp_calendar, bool workday) {
  if (state_ == State::kRunning) {
    LogWarn("alarm is running state, disable first");
    return false;
  }

  if (seconds_of_day < 0 || seconds_of_day >= kSecondsOfDay) {
    LogWarn("seconds_of_day:%d, out of range: [0,%d)", seconds_of_day, kSecondsOfDay);
    return false;
  }

  if (wp_calendar == nullptr) {
    LogWarn("wp_calendar is nullptr");
    return false;
  }

  seconds_of_day_ = seconds_of_day;
  wp_calendar_ = wp_calendar;
  workday_ = workday;

  state_ = State::kInited;
  return true;
}

int WorkdayAlarm::calculateWaitSeconds(uint32_t curr_local_ts) {
  int curr_days = curr_local_ts / kSecondsOfDay;
  int curr_seconds = curr_local_ts % kSecondsOfDay;

#if 1
  LogTrace("curr_days:%d, curr_seconds:%d", curr_days, curr_seconds);
#endif

  int wait_seconds = seconds_of_day_ - curr_seconds;
  for (int i = 0; i < 366; ++i) {
    if ((wait_seconds > 0) &&
        (workday_ == wp_calendar_->isWorkay(curr_days + i)))
      return wait_seconds;
    wait_seconds += kSecondsOfDay;
  }
  return -1;
}

bool WorkdayAlarm::onEnable() {
  wp_calendar_->subscribe(this);
  return true;
}

bool WorkdayAlarm::onDisable() {
  wp_calendar_->unsubscribe(this);
  return true;
}
}
}
