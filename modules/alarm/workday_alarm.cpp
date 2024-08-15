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
#include "workday_alarm.h"

#include <sys/time.h>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>

#include "workday_calendar.h"

namespace tbox {
namespace alarm {

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

bool WorkdayAlarm::calculateNextLocalTimeSec(uint32_t curr_local_ts, uint32_t &next_local_ts) {
  auto seconds_from_0000 = curr_local_ts % kSecondsOfDay;
  auto seconds_at_0000 = curr_local_ts - seconds_from_0000; //! 当地00:00的时间戳
  auto curr_days = curr_local_ts / kSecondsOfDay;

#if 1
  LogTrace("seconds_from_0000:%d, curr_days:%d", seconds_from_0000, curr_days);
#endif

  next_local_ts = seconds_at_0000 + seconds_of_day_;

  for (int i = 0; i < 367; ++i) {
    if ((curr_local_ts < next_local_ts) &&  //! 如果今天过了时间点，也不能算
        (workday_ == wp_calendar_->isWorkay(curr_days + i)))
      return true;

    next_local_ts += kSecondsOfDay;
  }

  return false;
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
