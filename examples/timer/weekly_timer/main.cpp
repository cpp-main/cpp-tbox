#include <iostream>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

#include <tbox/event/loop.h>
#include <tbox/timer/weekly_timer.h>

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

    LogOutput_Initialize(argv[0]);

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

    timer::WeeklyTimer tmr(sp_loop);
    tmr.initialize(seconds_from_00, week_mask);
    if (argc >= 4) {
      LogInfo("timezone:%d", timezone_offset_minutes);
      tmr.setTimezone(timezone_offset_minutes);
    }
    tmr.setCallback([]{ LogInfo("time is up"); });
    tmr.enable();

    sp_loop->runLoop(Loop::Mode::kForever);

    LogOutput_Cleanup();
    return 0;
}
