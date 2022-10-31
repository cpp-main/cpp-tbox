#include <iostream>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

#include <tbox/event/loop.h>
#include <tbox/timer/cron_timer.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void PrintUsage(const char *process_name) {
  cout << "Usage:" << process_name << " cron_expr [timezone_offset_minutes]" << endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    LogOutput_Initialize();

    std::string cron_expr;
    int timezone_offset_minutes = 0;

    try {
      cron_expr = argv[1];
      if (argc >= 3) {
        timezone_offset_minutes = std::stoi(argv[2]);
      }
    } catch (const std::exception &e) {
      PrintUsage(argv[0]);
      return 0;
    }

    LogInfo("cron_expr:%s", cron_expr.c_str());

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    timer::CronTimer tmr(sp_loop);
    tmr.initialize(cron_expr);
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
