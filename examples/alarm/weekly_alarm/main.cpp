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
#include <iostream>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

#include <tbox/event/loop.h>
#include <tbox/alarm/weekly_alarm.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void PrintUsage(const char *process_name) {
  cout << "Usage:" << process_name << " seconds_from_00 week_mask [timezone_offset_minutes]" << endl;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        PrintUsage(argv[0]);
        return 0;
    }

    LogOutput_Enable();

    int seconds_from_00 = 0;
    std::string week_mask;
    int timezone_offset_minutes = 0;

    try {
      seconds_from_00 = std::stoi(argv[1]);
      week_mask = argv[2];
      if (argc >= 4) {
        timezone_offset_minutes = std::stoi(argv[3]);
      }
    } catch (const std::exception &e) {
      PrintUsage(argv[0]);
      return 0;
    }

    LogInfo("second:%d, week_mask:%s", seconds_from_00, week_mask.c_str());

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    alarm::WeeklyAlarm tmr(sp_loop);
    tmr.initialize(seconds_from_00, week_mask);
    if (argc >= 4) {
      LogInfo("timezone:%d", timezone_offset_minutes);
      tmr.setTimezone(timezone_offset_minutes);
    }
    tmr.setCallback([]{ LogInfo("time is up"); });
    tmr.enable();

    sp_loop->runLoop(Loop::Mode::kForever);

    LogOutput_Disable();
    return 0;
}
