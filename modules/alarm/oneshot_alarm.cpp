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
#include "oneshot_alarm.h"

#include <sys/time.h>

#include <tbox/base/log.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace alarm {

bool OneshotAlarm::initialize(int seconds_of_day) {
  if (state_ == State::kRunning) {
    LogWarn("alarm is running state, disable first");
    return false;
  }

  if (seconds_of_day < 0 || seconds_of_day >= kSecondsOfDay) {
    LogWarn("seconds_of_day:%d, out of range: [0,%d)", seconds_of_day, kSecondsOfDay);
    return false;
  }

  seconds_of_day_ = seconds_of_day;
  state_ = State::kInited;
  return true;
}

bool OneshotAlarm::calculateNextLocalTimeSec(uint32_t curr_local_ts, uint32_t &next_local_ts) {
  auto seconds_from_0000 = curr_local_ts % kSecondsOfDay;
  auto seconds_at_0000 = curr_local_ts - seconds_from_0000; //! 当地00:00的时间戳

#if 1
  LogTrace("seconds_from_0000:%d", seconds_from_0000);
#endif

  next_local_ts = seconds_at_0000 + seconds_of_day_;
  if (curr_local_ts >= next_local_ts) //! 如果今天的时间已经过期了，则要延到明天
    next_local_ts += kSecondsOfDay;

  return true;
}

void OneshotAlarm::onTimeExpired() {
  state_ = State::kInited;

  ++cb_level_;
  if (cb_)
    cb_();
  --cb_level_;
}

}
}
