#include <iostream>
#include <thread>

#include <base/log.h>
#include <base/scope_exit.hpp>
#include <base/json.hpp>
#include <event/loop.h>
#include <event/signal_event.h>
#include <eventx/loop_wdog.h>
#include <util/pid_file.h>

#include "module.h"
#include "context_imp.h"
#include "args.h"
#include "log.h"

namespace tbox {
namespace main {

extern void InstallErrorSignals();
extern void UninstallErrorSignals();

extern void InstallTerminate();

extern void RegisterApps(Module &root, Context &ctx);
extern void SayHello();

extern std::function<void()> error_exit_func;

namespace {
struct Runtime {
  Log log;
  ContextImp ctx;
  Module apps;
  int loop_exit_wait = 1;
  std::thread thread;

  Runtime() : apps("", ctx) {
    RegisterApps(apps, ctx);
  }
};

Runtime* _runtime = nullptr;

void RunInBackend()
{
  auto loop = _runtime->ctx.loop();

  auto warn_signal = loop->newSignalEvent("main::RunInBackend::warn_signal");
  SetScopeExitAction([=] { delete warn_signal; });

  warn_signal->initialize({SIGPIPE, SIGHUP}, event::Event::Mode::kPersist);
  warn_signal->setCallback([](int signo) { LogWarn("Got signal %d", signo); });

  //! 启动前准备
  eventx::LoopWDog::Start();
  eventx::LoopWDog::Register(loop, "main");

  warn_signal->enable();

  LogDbg("Start!");
  loop->runLoop();
  LogDbg("Stoped");

  eventx::LoopWDog::Unregister(loop);
  eventx::LoopWDog::Stop();
}
}

bool Start(int argc, char **argv) {
  if (_runtime != nullptr) {
    std::cerr << "Err: process started" << std::endl;
    return false;
  }

  InstallErrorSignals();
  InstallTerminate();

  _runtime = new Runtime;

  auto &log = _runtime->log;
  auto &ctx = _runtime->ctx;
  auto &apps = _runtime->apps;

  Json js_conf;
  Args args(js_conf);

  log.fillDefaultConfig(js_conf);
  ctx.fillDefaultConfig(js_conf);
  apps.fillDefaultConfig(js_conf);

  if (!args.parse(argc, argv))
    return false;

  util::PidFile pid_file;
  if (js_conf.contains("pid_file")) {
    auto &js_pidfile = js_conf["pid_file"];
    if (js_pidfile.is_string()) {
      auto pid_filename = js_pidfile.get<std::string>();
      if (!pid_filename.empty()) {
        if (!pid_file.lock(js_pidfile.get<std::string>())) {
          std::cerr << "Warn: another process is running, exit" << std::endl;
          return false;
        }
      }
    }
  }

  if (js_conf.contains("loop_exit_wait")) {
    auto js_loop_exit_wait = js_conf.at("loop_exit_wait");
    if (js_loop_exit_wait.is_number()) {
      _runtime->loop_exit_wait = js_loop_exit_wait.get<int>();
    } else {
      std::cerr << "Warn: loop_exit_wait invaild" << std::endl;
    }
  }

  log.initialize(argv[0], ctx, js_conf);

  SayHello();

  error_exit_func = [&] {
    log.cleanup();
  };

  if (ctx.initialize(js_conf)) {
    if (apps.initialize(js_conf)) {
      if (ctx.start()) {  //! 启动所有应用
        if (apps.start()) {
          _runtime->thread = std::thread(RunInBackend);
          return true;
        } else {
          LogErr("App start fail");
        }
        ctx.stop();
      } else {
        LogErr("Ctx start fail");
      }
      apps.cleanup();
    } else {
      LogErr("Apps init fail");
    }
    ctx.cleanup();
  } else {
    LogErr("Context init fail");
  }

  return false;
}

void Stop() {
  if (_runtime == nullptr) {
    std::cerr << "Err: process not start" << std::endl;
    return;
  }

  _runtime->ctx.loop()->runInLoop(
    [] {
      _runtime->apps.stop();
      _runtime->ctx.stop();
      _runtime->ctx.loop()->exitLoop(std::chrono::seconds(_runtime->loop_exit_wait));
      LogDbg("Loop will exit after %d sec", _runtime->loop_exit_wait);
    },
    "main::Stop"
  );
  _runtime->thread.join();

  _runtime->apps.cleanup();  //! cleanup所有应用
  _runtime->ctx.cleanup();

  LogInfo("Bye!");
  _runtime->log.cleanup();

  CHECK_DELETE_RESET_OBJ(_runtime);

  UninstallErrorSignals();
}

}
}
