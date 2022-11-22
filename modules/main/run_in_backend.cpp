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
  int loop_exit_wait = 0;
  std::thread thread;

  Runtime() : apps("", ctx) {}
};

std::shared_ptr<Runtime> runtime = nullptr;

void RunInThread()
{
  auto loop = runtime->ctx.loop();

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
  if (runtime) {
    std::cerr << "Err: process started" << std::endl;
    return false;
  }

  runtime = std::make_shared<Runtime>();

  auto &log = runtime->log;
  auto &ctx = runtime->ctx;
  auto &apps = runtime->apps;

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
      runtime->loop_exit_wait = js_loop_exit_wait.get<int>();
    } else {
      std::cerr << "Warn: loop_exit_wait invaild" << std::endl;
    }
  }

  log.initialize(argv[0], ctx, js_conf);

  LogInfo("Wellcome!");

  if (!ctx.initialize(js_conf)) {
    LogErr("Context init fail");
    return false;
  }

  if (!apps.initialize(js_conf)) {
    LogErr("Apps init fail");
    return false;
  }

  if (!ctx.start() || !apps.start()) {  //! 启动所有应用
    LogErr("Apps start fail");
    return false;
  }

  runtime->thread = std::thread(RunInThread);
  return true;
}

void Stop() {
  if (!runtime) {
    std::cerr << "Err: process not start" << std::endl;
    return;
  }

  runtime->ctx.loop()->exitLoop(std::chrono::seconds(runtime->loop_exit_wait));
  runtime->thread.join();

  runtime->apps.cleanup();  //! cleanup所有应用
  runtime->ctx.cleanup();
  LogInfo("Bye!");
}

}
}
