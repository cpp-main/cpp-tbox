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
constexpr auto kMaxTimezoneOffsetMinutes = 60 * 12;
}

WeeklyTimer::WeeklyTimer(event::Loop *wp_loop) :
  wp_loop_(wp_loop),
  sp_timer_ev_(wp_loop->newTimerEvent())
{
  sp_timer_ev_->setCallback(
    [this] {
      state_ = State::kInited;
      if (!week_mask_.empty())
        activeTimer();

      ++cb_level_;
      if (cb_)
        cb_();
      --cb_level_;
    }
  );
}

WeeklyTimer::~WeeklyTimer() {
  assert(cb_level_ == 0);
  cleanup();
  delete sp_timer_ev_;
}

bool WeeklyTimer::initialize(int seconds_of_day, const std::string &week_mask, int timezone_offset_minutes) {
  if (state_ == State::kRunning) {
    LogWarn("timer is running state, disable first");
    return false;
  }

  if (seconds_of_day < 0 || seconds_of_day >= kSecondsOfDay) {
    LogWarn("seconds_of_day:%d, out of range: [0,%d)", seconds_of_day, kSecondsOfDay);
    return false;
  }

  if (week_mask.size() != 7 && !week_mask.empty()) {
    LogWarn("week_mask:%s, length either 0 or 7", week_mask.c_str());
    return false;
  }

  week_mask_ = week_mask;
  seconds_of_day_ = seconds_of_day;
  timezone_offset_seconds_ = timezone_offset_minutes * 60;
  state_ = State::kInited;

  return true;
}

bool WeeklyTimer::isEnabled() const {
  return state_ == State::kRunning;
}

bool WeeklyTimer::enable() {
  if (state_ == State::kInited)
    return activeTimer();

  LogWarn("should initialize first");
  return false;
}

bool WeeklyTimer::disable() {
  if (state_ == State::kRunning) {
    state_ = State::kInited;
    return sp_timer_ev_->disable();
  }
  return false;
}

void WeeklyTimer::cleanup() {
  if (state_ < State::kInited)
    return;

  disable();

  week_mask_.clear();
  seconds_of_day_ = 0;
  timezone_offset_seconds_ = 0;
  cb_ = nullptr;
  state_ = State::kNone;
}

int WeeklyTimer::calculateWaitSeconds(uint32_t curr_utc_ts) {
  uint32_t curr_local_ts = curr_utc_ts + timezone_offset_seconds_;

  int curr_week = (((curr_local_ts % kSecondsOfWeek) / kSecondsOfDay) + 4) % 7;
  int curr_seconds = curr_local_ts % kSecondsOfDay;

#if 0
  LogTrace("curr_week:%d, curr_seconds:%d", curr_week, curr_seconds);
#endif

  int wait_seconds = seconds_of_day_ - curr_seconds;
  if (week_mask_.empty()) {
    if (wait_seconds <= 0)
      wait_seconds += kSecondsOfDay;
    return wait_seconds;
  } else {
    for (int i = 0; i < 7; ++i) {
      if (wait_seconds > 0 && week_mask_.at((i + curr_week) % 7) == '1')
        return wait_seconds;
      wait_seconds += kSecondsOfDay;
    }
    return -1;
  }
}

bool WeeklyTimer::activeTimer() {
  struct timeval tv;
  int ret = gettimeofday(&tv, nullptr);
  if (ret != 0) {
    LogWarn("gettimeofday() fail, ret:%d", ret);
    return false;
  }

  auto wait_seconds = calculateWaitSeconds(tv.tv_sec);
#if 0
  LogTrace("wait_seconds:%d", wait_seconds);
#endif
  if (wait_seconds < 0)
    return false;

  auto wait_milliseconds = wait_seconds * 1000 - tv.tv_usec / 1000;
  sp_timer_ev_->initialize(std::chrono::milliseconds(wait_milliseconds), event::Event::Mode::kOneshot);
  sp_timer_ev_->enable();

  state_ = State::kRunning;
  return true;
}

}
}
