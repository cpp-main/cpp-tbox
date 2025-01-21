/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "alarm.h"

#include <sys/time.h>
#include <cstring>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace alarm {

bool Alarm::GetCurrentUtcTime(uint32_t &utc_sec)
{
  struct timeval utc_tv;
  if (gettimeofday(&utc_tv, nullptr) == 0) {
    utc_sec = utc_tv.tv_sec;
    return true;
  }

  LogErrno(errno, "gettimeofday fail");
  return false;
}

bool Alarm::GetCurrentUtcTime(uint32_t &utc_sec, uint32_t &utc_usec)
{
  struct timeval utc_tv;
  if (gettimeofday(&utc_tv, nullptr) == 0) {
    utc_sec = utc_tv.tv_sec;
    utc_usec = utc_tv.tv_usec;
    return true;
  }

  LogErrno(errno, "gettimeofday fail");
  return false;
}

Alarm::Alarm(event::Loop *wp_loop) :
  wp_loop_(wp_loop),
  sp_timer_ev_(wp_loop->newTimerEvent("Alarm::sp_timer_ev_"))
{
  sp_timer_ev_->setCallback([this] { onTimeExpired(); });
}

Alarm::~Alarm() {
  TBOX_ASSERT(cb_level_ == 0); //!< 防止回调中析构

  cleanup();
  delete sp_timer_ev_;
}

void Alarm::setTimezone(int offset_minutes) {
  timezone_offset_seconds_ = offset_minutes * 60;
  using_independ_timezone_ = true;
}

bool Alarm::isEnabled() const {
  return state_ == State::kRunning;
}

bool Alarm::enable() {
  if (state_ == State::kInited) {
    if (onEnable())
      return activeTimer();
  }

  LogWarn("should initialize first");
  return false;
}

bool Alarm::disable() {
  if (state_ == State::kRunning) {
    if (onDisable()) {
      state_ = State::kInited;
      return sp_timer_ev_->disable();
    }
  }
  return false;
}

void Alarm::cleanup() {
  if (state_ < State::kInited)
    return;

  disable();

  cb_ = nullptr;
  using_independ_timezone_ = false;
  timezone_offset_seconds_ = 0;
  state_ = State::kNone;
  target_utc_sec_ = 0;
}

void Alarm::refresh() {
  if (state_ == State::kRunning) {
    state_ = State::kInited;
    sp_timer_ev_->disable();
    target_utc_sec_ = 0;    //! 如果不清0，那么每refresh()一次都会往后延一天
    activeTimer();
  }
}

uint32_t Alarm::remainSeconds() const {
  uint32_t curr_utc_sec = 0;
  if (state_ == State::kRunning && GetCurrentUtcTime(curr_utc_sec)) {
    return target_utc_sec_ - curr_utc_sec;
  }
  return 0;
}

namespace {
//! 获取系统的时区偏移秒数
int GetSystemTimezoneOffsetSeconds() {
#if defined(__MINGW32__) || defined(_MSC_VER) || defined(_WIN32)
#if defined(__MINGW32__) && !__has_include(<_mingw_stat64.h>)
  LogErr("can't get timezone offset, not support.");
  return 0;
#else
  long tm_gmtoff = 0;
#if (defined(_MSC_VER) || defined(_UCRT)) && !defined(__BIONIC__)
  {
    errno_t errn = _get_timezone(&tm_gmtoff);
    if (errn != 0) {
      LogErr("_get_timezone() error:%d", errn);
      return 0;
    }
  }
#elif defined(_WIN32) && !defined(__BIONIC__) && !defined(__WINE__) && !defined(__CYGWIN__)
  tm_gmtoff = _timezone;
#else
  tm_gmtoff = timezone;
#endif
  return static_cast<int>(tm_gmtoff);
#endif
#else
  //! 假设当前0时区的时间是 1970-1-1 12:00，即 utc_ts = 12 * 3600
  //! 通过 localtime_r() 获取本地的时间 local_tm。
  //! 通过 local_tm 中的 hour, min, sec 可计算出本地的时间戳 local_ts。
  //! 再用本地的时间戳减去 utc_ts 即可得出期望的值。
  //
  //! 为什么选用0时区的12时，而不是其它时间点呢？
  //! 因为在这个时间点上，计算出的任何一个时间的时间都是在 00:00 ~ 23:59 之间的
  struct tm local_tm;
  time_t utc_ts = 12 * 3600;
  localtime_r(&utc_ts, &local_tm);
  int local_ts = local_tm.tm_hour * 3600 + local_tm.tm_min * 60 + local_tm.tm_sec;
  return (local_ts - static_cast<int>(utc_ts));
#endif
}
}

bool Alarm::activeTimer() {
  uint32_t curr_utc_sec, curr_utc_usec;
  if (!GetCurrentUtcTime(curr_utc_sec, curr_utc_usec))
    return false;

  int timezone_offset_seconds = using_independ_timezone_ ? \
                                timezone_offset_seconds_ : GetSystemTimezoneOffsetSeconds();

  auto next_utc_start_sec = std::max(curr_utc_sec, target_utc_sec_);

  //! Q: 为什么要用curr_utc_sec与target_utc_sec_中最大值来算下一轮的时间点？
  //! A: 因为在实践中存在steady_clock比system_clock快的现象，会导致重复触发定时任务的问题。
  //!    比如：定的时间为每天10:00:00.000触发，结果定时任务在09:59:59.995就触发了。如果下
  //!    一轮的时间计算是从09:59:59:995计算，它会发现下一次的触发时间点在5ms之后。于是5ms
  //!    之后就再次触发一次。
  //!    解决办法就是：除了第一次按当前的时间算外，后面的如果出现提前触发的，按期望的算。
  //!    就如上面的例子，就算是提前触发了，后面的按10:00:00.000计算。从而避免重复触发问题

  uint32_t next_local_start_sec = next_utc_start_sec + timezone_offset_seconds;
  uint32_t next_local_sec = 0;

  if (!calculateNextLocalTimeSec(next_local_start_sec, next_local_sec))
    return false;

  uint32_t next_utc_sec = next_local_sec - timezone_offset_seconds;

  auto remain_sec = next_utc_sec - curr_utc_sec;
  //! 提升精度，计算中需要等待的毫秒数
  auto remain_usec = (remain_sec * 1000) - (curr_utc_usec / 1000);

#if 1
  LogTrace("next_utc_sec:%u, remain_sec:%u, remain_usec:%u", next_utc_sec, remain_sec, remain_usec);
#endif

  //! 启动定时器
  sp_timer_ev_->initialize(std::chrono::milliseconds(remain_usec), event::Event::Mode::kOneshot);
  sp_timer_ev_->enable();

  state_ = State::kRunning;
  target_utc_sec_ = next_utc_sec;
  return true;
}

void Alarm::onTimeExpired() {
#if 1
  LogTrace("time expired, target_utc_sec:%u", target_utc_sec_);
#endif

  state_ = State::kInited;
  activeTimer();

  RECORD_SCOPE();
  ++cb_level_;
  if (cb_)
    cb_();
  --cb_level_;
}

}
}
