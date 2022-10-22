#include "timer.h"

#include <sys/time.h>
#include <cassert>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace timer {

Timer::Timer(event::Loop *wp_loop) :
  wp_loop_(wp_loop),
  sp_timer_ev_(wp_loop->newTimerEvent())
{
  sp_timer_ev_->setCallback(
    [this] {
      state_ = State::kInited;
      activeTimer();

      ++cb_level_;
      if (cb_)
        cb_();
      --cb_level_;
    }
  );
}

Timer::~Timer() {
  assert(cb_level_ == 0);
  cleanup();
  delete sp_timer_ev_;
}

void Timer::setTimezone(int offset_minutes) {
  timezone_offset_seconds_ = offset_minutes * 60;
  using_independ_timezone_ = true;
}

bool Timer::isEnabled() const {
  return state_ == State::kRunning;
}

bool Timer::enable() {
  if (state_ == State::kInited) {
    if (onEnable())
      return activeTimer();
  }

  LogWarn("should initialize first");
  return false;
}

bool Timer::disable() {
  if (state_ == State::kRunning) {
    if (onDisable()) {
      state_ = State::kInited;
      return sp_timer_ev_->disable();
    }
  }
  return false;
}

void Timer::cleanup() {
  if (state_ < State::kInited)
    return;

  onCleanup();
  disable();

  cb_ = nullptr;
  using_independ_timezone_ = false;
  timezone_offset_seconds_ = 0;
  state_ = State::kNone;
}

int GetSystemTimezoneOffsetSeconds() {
  time_t utc_ts = 12 * 3600;
  struct tm local_tm;
  localtime_r(&utc_ts, &local_tm);
  int local_ts = local_tm.tm_hour * 3600 + local_tm.tm_min * 60 + local_tm.tm_sec;
  return (local_ts - static_cast<int>(utc_ts));
}

bool Timer::activeTimer() {
  LogTag();
  struct timeval tv;
  int ret = gettimeofday(&tv, nullptr);
  if (ret != 0) {
    LogWarn("gettimeofday() fail, ret:%d", ret);
    return false;
  }

  int timezone_offset_seconds = using_independ_timezone_ ? \
                                timezone_offset_seconds_ : GetSystemTimezoneOffsetSeconds();
  auto wait_seconds = calculateWaitSeconds(tv.tv_sec + timezone_offset_seconds);
#if 1
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
