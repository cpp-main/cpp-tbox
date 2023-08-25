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

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace alarm {

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
  target_utc_ts_ = 0;
}

void Alarm::refresh() {
  if (state_ == State::kRunning) {
    state_ = State::kInited;
    sp_timer_ev_->disable();
    activeTimer();
  }
}

uint32_t Alarm::remainSeconds() const {
  if (state_ == State::kRunning) {
    struct timeval utc_tv;
    int ret = gettimeofday(&utc_tv, nullptr);
    if (ret == 0)
      return target_utc_ts_ - utc_tv.tv_sec;
  }
  return 0;
}

namespace {
//! 获取系统的时区偏移秒数
int GetSystemTimezoneOffsetSeconds() {
	long tm_gmtoff{};
#if (defined(_MSC_VER) || defined(_UCRT)) && !defined(__BIONIC__)
	{
		errno_t errn = _get_timezone(&tm_gmtoff);
		if (errn)
			return 0;
	}
#elif defined(_WIN32) && !defined(__BIONIC__) && !defined(__WINE__) && !defined(__CYGWIN__)
	tm_gmtoff = _timezone;
#else
	tm_gmtoff = timezone;
#endif
	return static_cast<int>(tm_gmtoff);
}
}

bool Alarm::activeTimer() {
  //! 使用 gettimeofday() 获取当前0时区的时间戳，精确到微秒
  struct timeval utc_tv;
  int ret = gettimeofday(&utc_tv, nullptr);
  if (ret != 0) {
    LogWarn("gettimeofday() fail, ret:%d", ret);
    return false;
  }

  int timezone_offset_seconds = using_independ_timezone_ ? \
                                timezone_offset_seconds_ : GetSystemTimezoneOffsetSeconds();
  //! 计算需要等待的秒数
  auto wait_seconds = calculateWaitSeconds(utc_tv.tv_sec + timezone_offset_seconds);
#if 1
  LogTrace("wait_seconds:%d", wait_seconds);
#endif
  if (wait_seconds < 0)
    return false;

  //! 提升精度，计算中需要等待的毫秒数
  auto wait_milliseconds = wait_seconds * 1000 - utc_tv.tv_usec / 1000;
  //! 启动定时器
  sp_timer_ev_->initialize(std::chrono::milliseconds(wait_milliseconds), event::Event::Mode::kOneshot);
  sp_timer_ev_->enable();

  state_ = State::kRunning;
  target_utc_ts_ = utc_tv.tv_sec + wait_seconds;
  return true;
}

void Alarm::onTimeExpired() {
  state_ = State::kInited;
  activeTimer();

  ++cb_level_;
  if (cb_)
    cb_();
  --cb_level_;
}

}
}
