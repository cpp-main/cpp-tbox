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
#include "workday_calendar.h"

#include <algorithm>
#include <tbox/base/assert.h>

#include "alarm.h"

namespace tbox {
namespace alarm {

void WorkdayCalendar::updateSpecialDays(const std::map<int, bool> &special_days) {
  special_days_ = special_days;
  for (auto alarm : watch_alarms_)
    alarm->refresh();
}

void WorkdayCalendar::updateWeekMask(uint8_t week_mask) {
  week_mask_ = week_mask;
  for (auto alarm : watch_alarms_)
    alarm->refresh();
}

void WorkdayCalendar::subscribe(Alarm *alarm) {
  TBOX_ASSERT(alarm != nullptr);
  watch_alarms_.push_back(alarm);
}

void WorkdayCalendar::unsubscribe(Alarm *alarm) {
  TBOX_ASSERT(alarm != nullptr);
  auto iter = std::remove(watch_alarms_.begin(), watch_alarms_.end(), alarm);
  watch_alarms_.erase(iter, watch_alarms_.end());
}

bool WorkdayCalendar::isWorkay(int day_index) const {
  auto iter = special_days_.find(day_index);
  if (iter != special_days_.end()) {
    return iter->second;
  } else {
    int curr_week = ((day_index % 7) + 4) % 7;
    return (week_mask_ & (1 << curr_week));
  }
}

}
}
