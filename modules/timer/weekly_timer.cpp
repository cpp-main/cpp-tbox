#include "weekly_timer.h"

#include <sys/time.h>
#include <cassert>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace timer {

namespace {
constexpr auto kSecondsOfDay = 60 * 60 * 24;
constexpr auto kSecondsOfWeek = kSecondsOfDay * 7;
}

bool WeeklyTimer::initialize(int seconds_of_day, const std::string &week_mask) {
  if (state_ == State::kRunning) {
    LogWarn("timer is running state, disable first");
    return false;
  }

  if (seconds_of_day < 0 || seconds_of_day >= kSecondsOfDay) {
    LogWarn("seconds_of_day:%d, out of range: [0,%d)", seconds_of_day, kSecondsOfDay);
    return false;
  }

  if (week_mask.size() != 7) {
    LogWarn("week_mask:%s, length should be 7", week_mask.c_str());
    return false;
  }

  week_mask_ = 0;
  for (int i = 0; i < 7; ++i) {
    if (week_mask.at(i) == '1')
      week_mask_ |= (1 << i);
  }

  seconds_of_day_ = seconds_of_day;
  state_ = State::kInited;
  return true;
}

int WeeklyTimer::calculateWaitSeconds(uint32_t curr_local_ts) {
  int curr_week = (((curr_local_ts % kSecondsOfWeek) / kSecondsOfDay) + 4) % 7;
  int curr_seconds = curr_local_ts % kSecondsOfDay;

#if 1
  LogTrace("curr_week:%d, curr_seconds:%d", curr_week, curr_seconds);
#endif

  int wait_seconds = seconds_of_day_ - curr_seconds;
  for (int i = 0; i < 7; ++i) {
    int week = (i + curr_week) % 7;
    if (wait_seconds > 0 && (week_mask_ & (1 << week)))
      return wait_seconds;
    wait_seconds += kSecondsOfDay;
  }
  return -1;
}

}
}
