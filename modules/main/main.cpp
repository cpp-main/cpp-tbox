#include <vector>
#include <iostream>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/base/json.hpp>

#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/event/signal_event.h>

#include <tbox/util/thread_wdog.h>
#include <tbox/util/pid_file.h>
#include <tbox/util/fs.h>

#include "module.h"
#include "context_imp.h"
#include "args.h"
#include "log.h"

namespace tbox {
namespace main {

extern void RegisterSignals();
extern void RegisterApps(Module &root, Context &ctx);
extern void Run(ContextImp &ctx, Module &apps, int loop_exit_wait);

std::function<void()> error_exit_func;  //!< 出错异常退出前要做的事件

class Apps : public Module {
  public:
    Apps(Context &ctx) : Module("", ctx) {
        RegisterApps(*this, ctx);
    }
};

int Main(int argc, char **argv)
{
    RegisterSignals();

    Log log;
    ContextImp ctx;
    Apps apps(ctx);

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
                    return 0;
                }
            }
        }
    }

    int loop_exit_wait = 1;
    if (js_conf.contains("loop_exit_wait")) {
        auto js_loop_exit_wait = js_conf.at("loop_exit_wait");
        if (js_loop_exit_wait.is_number()) {
            loop_exit_wait = js_loop_exit_wait.get<int>();
        } else {
            std::cerr << "Warn: loop_exit_wait invaild" << std::endl;
        }
    }

    log.initialize(argv[0], ctx, js_conf);

    LogInfo("Wellcome!");

    //! 注册异常退出时的动作，在异常信号触发时调用
    error_exit_func = [&] {
        //! 主要是保存日志
        log.cleanup();
    };

    if (ctx.initialize(js_conf)) {
        if (apps.initialize(js_conf)) {
            if (ctx.start() && apps.start()) {  //! 启动所有应用
                Run(ctx, apps, loop_exit_wait);
            } else {
                LogErr("Apps start fail");
            }
            apps.cleanup();  //! cleanup所有应用
        } else {
            LogErr("Apps init fail");
        }
        ctx.cleanup();
    } else {
        LogErr("Context init fail");
    }

    LogInfo("Bye!");
    return 0;
}

void Run(ContextImp &ctx, Module &apps, int loop_exit_wait)
{
    auto feeddog_timer = ctx.loop()->newTimerEvent();
    auto stop_signal   = ctx.loop()->newSignalEvent();

    //! 预定在离开时自动释放对象，确保无内存泄漏
    SetScopeExitAction(
        [=] {
            delete stop_signal;
            delete feeddog_timer;
        }
    );

    stop_signal->initialize({SIGINT, SIGTERM}, event::Event::Mode::kOneshot);
    stop_signal->setCallback(
        [&] (int signo) {
            LogInfo("Got signal %d", signo);
            apps.stop();
            ctx.stop();
            ctx.loop()->exitLoop(std::chrono::seconds(loop_exit_wait));
            LogInfo("Loop will exit after %d sec", loop_exit_wait);
        }
    );

    //! 创建喂狗定时器
    feeddog_timer->initialize(std::chrono::seconds(2), event::Event::Mode::kPersist);
    feeddog_timer->setCallback(util::ThreadWDog::FeedDog);

    //! 启动前准备
    util::ThreadWDog::Start();
    util::ThreadWDog::Register("main", 3);

    feeddog_timer->enable();
    stop_signal->enable();

    LogInfo("Start!");

    try {
        ctx.loop()->runLoop();
    } catch (const std::exception &e) {
        LogErr("catch execption: %s", e.what());
    } catch (...) {
        LogErr("catch unknown execption");
    }

    LogInfo("Stoped");

    util::ThreadWDog::Unregister();
    util::ThreadWDog::Stop();
}

}
}

__attribute__((weak))
//! 定义为弱定义，允许用户自己定义。
//! 另一方面，避免在 make test 时与 gtest 的 main() 冲突。
int main(int argc, char **argv)
{
    return tbox::main::Main(argc, argv);
}
