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
#include "weekly_alarm.h"

#include <sys/time.h>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace alarm {

bool WeeklyAlarm::initialize(int seconds_of_day, const std::string &week_mask) {
  if (state_ == State::kRunning) {
    LogWarn("alarm is running state, disable first");
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

bool WeeklyAlarm::calculateNextLocalTimeSec(uint32_t curr_local_ts, uint32_t &next_local_ts) {
  auto seconds_from_0000 = curr_local_ts % kSecondsOfDay;
  auto seconds_at_0000 = curr_local_ts - seconds_from_0000; //! 当地00:00的时间戳
  auto curr_week = (((curr_local_ts % kSecondsOfWeek) / kSecondsOfDay) + 4) % 7;

#if 1
  LogTrace("seconds_from_0000:%d, curr_week:%d", seconds_from_0000, curr_week);
#endif

  next_local_ts = seconds_at_0000 + seconds_of_day_;

  for (int i = 0; i < 8; ++i) {
    int week = (i + curr_week) % 7;

    if ((curr_local_ts < next_local_ts) &&  //! 如果今天过了时间点，也不能算
        (week_mask_ & (1 << week)))
      return true;

    next_local_ts += kSecondsOfDay;
  }

  return false;
}

}
}
