#include <iostream>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

#include <tbox/event/loop.h>
#include <tbox/timer/crontab_timer.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void PrintUsage(const char *process_name) {
  cout << "Usage:" << process_name << " crontab_expr [timezone_offset_minutes]" << endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    LogOutput_Initialize(argv[0]);

    std::string crontab_expr;
    int timezone_offset_minutes = 0;

    try {
      crontab_expr = argv[1];
      if (argc >= 3) {
        timezone_offset_minutes = std::stoi(argv[2]);
      }
    } catch (const std::exception &e) {
      PrintUsage(argv[0]);
      return 0;
    }

    LogInfo("crontab_expr:%s", crontab_expr.c_str());

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    timer::CrontabTimer tmr(sp_loop);
    tmr.initialize(crontab_expr);
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
