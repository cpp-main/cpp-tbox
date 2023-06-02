#include <iostream>

#include <base/log.h>
#include <base/log_output.h>
#include <base/scope_exit.hpp>

#include <event/loop.h>
#include <alarm/oneshot_alarm.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void PrintUsage(const char *process_name) {
  cout << "Usage:" << process_name << " seconds_from_00 [timezone_offset_minutes]" << endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    LogOutput_Initialize();

    int seconds_from_00 = 0;
    int timezone_offset_minutes = 0;

    try {
      seconds_from_00 = std::stoi(argv[1]);
      if (argc >= 3) {
        timezone_offset_minutes = std::stoi(argv[3]);
      }
    } catch (const std::exception &e) {
      PrintUsage(argv[0]);
      return 0;
    }

    LogInfo("second:%d", seconds_from_00);

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    alarm::OneshotAlarm tmr(sp_loop);
    tmr.initialize(seconds_from_00);
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
