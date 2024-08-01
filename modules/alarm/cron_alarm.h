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
#ifndef TBOX_ALARM_CRON_ALARM_H
#define TBOX_ALARM_CRON_ALARM_H

#include <string>

#include "alarm.h"

namespace tbox {
namespace alarm {

/*
 * @brief The linux cron alarm.
 *
 * CronAlarm allow the user to make plans by linux cron expression.
 *  Linux-cron tab expression
  *    *    *    *    *    *
  -    -    -    -    -    -
  |    |    |    |    |    |
  |    |    |    |    |    +----- day of week (0 - 7) (Sunday=0 or 7) OR sun,mon,tue,wed,thu,fri,sat
  |    |    |    |    +---------- month (1 - 12) OR jan,feb,mar,apr ...
  |    |    |    +--------------- day of month (1 - 31)
  |    |    +-------------------- hour (0 - 23)
  |    +------------------------- minute (0 - 59)
  +------------------------------ second (0 - 59)

 *
 * code example:
 *
 * Loop *loop = Loop::New();
 *
 * CronAlarm tmr(loop);
 * tmr.initialize("18 28 14 * * *"); // every day at 14:28:18
 * tmr.setCallback([] { std::cout << "timeout" << std::endl; });
 * tmr.enable();
 *
 * loop->runLoop();
 */
class CronAlarm : public Alarm
{
  public:
    explicit CronAlarm(event::Loop *wp_loop);
    virtual ~CronAlarm();

    bool initialize(const std::string &cron_expr_str);

  protected:
    virtual bool calculateNextLocalTimeSec(uint32_t curr_local_sec, uint32_t &next_local_sec) override;

  private:
    void *sp_cron_expr_;
};

}
}

#endif //TBOX_ALARM_CRON_ALARM_H
