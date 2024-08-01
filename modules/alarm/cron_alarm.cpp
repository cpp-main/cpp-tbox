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
#include "cron_alarm.h"

#include <cstring>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/event/loop.h>

#include "3rd-party/ccronexpr.h"

namespace tbox {
namespace alarm {

CronAlarm::CronAlarm(event::Loop *loop) :
  Alarm(loop),
  sp_cron_expr_(malloc(sizeof(cron_expr)))
{
  TBOX_ASSERT(sp_cron_expr_ != nullptr);
  memset(sp_cron_expr_, 0, sizeof(cron_expr));
}

CronAlarm::~CronAlarm()
{
  free(sp_cron_expr_);
}

bool CronAlarm::initialize(const std::string &cron_expr_str)
{
  if (state_ == State::kRunning) {
    LogWarn("alarm is running state, disable first");
    return false;
  }

  const char *error_str = nullptr;
  memset(sp_cron_expr_, 0, sizeof(cron_expr));

  // check validity of cron str
  cron_parse_expr(cron_expr_str.c_str(), static_cast<cron_expr *>(sp_cron_expr_), &error_str);
  if (error_str != nullptr) { // Invalid expression.
    LogWarn("cron_expr error: %s", error_str);
    return false;
  }

  state_ = State::kInited;
  return true;
}

bool CronAlarm::calculateNextLocalTimeSec(uint32_t curr_local_ts, uint32_t &next_local_ts) {
  next_local_ts = cron_next(static_cast<cron_expr *>(sp_cron_expr_), curr_local_ts);
  return true;
}

}
}
