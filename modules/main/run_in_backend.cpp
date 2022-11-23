#include <iostream>
#include <thread>

#include <tbox/base/log.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/json.hpp>
#include <tbox/event/loop.h>
#include <tbox/eventx/loop_wdog.h>
#include <tbox/util/pid_file.h>

#include "module.h"
#include "context_imp.h"
#include "args.h"
#include "log.h"

namespace tbox {
namespace main {

extern void RegisterApps(Module &root, Context &ctx);

namespace {
struct Runtime {
  Log log;
  ContextImp ctx;
  Module apps;
  int loop_exit_wait = 1;
  std::thread thread;

  Runtime() : apps("", ctx) {}
};

std::shared_ptr<Runtime> _runtime;

void RunInBackend()
{
  auto loop = _runtime->ctx.loop();

  //! 启动前准备
  eventx::LoopWDog::Start();
  eventx::LoopWDog::Register(loop, "main");

  LogDbg("Start!");

  try {
    loop->runLoop();
  } catch (const std::exception &e) {
    LogErr("catch execption: %s", e.what());
  } catch (...) {
    LogErr("catch unknown execption");
  }

  LogDbg("Stoped");

  eventx::LoopWDog::Unregister(loop);
  eventx::LoopWDog::Stop();
}
}

bool Start(int argc, char **argv) {
  if (_runtime) {
    std::cerr << "Err: process started" << std::endl;
    return false;
  }

  _runtime = std::make_shared<Runtime>();

  auto &log = _runtime->log;
  auto &ctx = _runtime->ctx;
  auto &apps = _runtime->apps;

  Json js_conf;
  Args args(js_conf);

  log.fillDefaultConfig(js_conf);
  ctx.fillDefaultConfig(js_conf);
  apps.fillDefaultConfig(js_conf);

  if (!args.parse(argc, argv))
    return 0;

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

  LogInfo("Wellcome!");

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
  if (!_runtime) {
    std::cerr << "Err: process not start" << std::endl;
    return;
  }

  _runtime->ctx.loop()->runInLoop(
    [] {
      _runtime->apps.stop();
      _runtime->ctx.stop();
      _runtime->ctx.loop()->exitLoop(std::chrono::seconds(_runtime->loop_exit_wait));
    }
  );
  _runtime->thread.join();

  _runtime->apps.cleanup();  //! cleanup所有应用
  _runtime->ctx.cleanup();

  LogInfo("Bye!");
  _runtime.reset();
}

}
}
