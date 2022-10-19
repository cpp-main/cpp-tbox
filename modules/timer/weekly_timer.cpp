#include "weekly_timer.h"

#include <sys/time.h>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace timer {

WeeklyTimer::WeeklyTimer(event::Loop *wp_loop) :
  wp_loop_(wp_loop),
  sp_timer_ev_(wp_loop->newTimerEvent())
{
  sp_timer_ev_->setCallback(
    [this] {
      state_ = State::kInited;
      if (!week_mask_.empty())
        activeTimer();

      if (cb_)
        cb_();
    }
  );
}

WeeklyTimer::~WeeklyTimer() {
  cleanup();
  delete sp_timer_ev_;
}

bool WeeklyTimer::initialize(int seconds_of_day, const std::string &week_mask, int timezone_offset_minutes) {
  if (state_ == State::kRunning) {
    LogWarn("timer is running state, disable first");
    return false;
  }

  if (seconds_of_day < 0) {
    LogWarn("seconds_of_day:%d, < 0", seconds_of_day);
    return false;
  }

  if (week_mask_.size() != 7 && !week_mask_.empty()) {
    LogWarn("week_mask:%s, invalid", week_mask.c_str());
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

  LogWarn("need initialize first");
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

namespace {
constexpr auto kSecondsOfDay = 60 * 60 * 24;
constexpr auto kSecondsOfWeek = kSecondsOfDay * 7;

uint32_t GetCurrentUTCSeconds() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec;
}
}

int WeeklyTimer::calculateWaitSeconds() {
  uint32_t curr_utc_ts = GetCurrentUTCSeconds();
  uint32_t curr_local_ts = curr_utc_ts + timezone_offset_seconds_;

  int curr_week = (((curr_local_ts % kSecondsOfWeek) / kSecondsOfDay) + 4) % 7;
  int curr_seconds = curr_local_ts % kSecondsOfDay;

  LogTrace("curr_week:%d, curr_seconds:%d", curr_week, curr_seconds);

  int wait_seconds = static_cast<int>(seconds_of_day_) - curr_seconds;
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
  auto wait_seconds = calculateWaitSeconds();
  LogTrace("wait_seconds:%d", wait_seconds);
  if (wait_seconds < 0)
    return false;

  sp_timer_ev_->initialize(std::chrono::seconds(wait_seconds), event::Event::Mode::kOneshot);
  sp_timer_ev_->enable();

  state_ = State::kRunning;
  return true;
}

}
}
